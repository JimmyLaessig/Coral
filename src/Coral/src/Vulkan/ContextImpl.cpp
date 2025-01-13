#include "ContextImpl.hpp"

#include "../BufferPool.hpp"
#include "DescriptorSetPool.hpp"
#include "CommandQueueImpl.hpp"
#include "BufferImpl.hpp"
#include "BufferViewImpl.hpp"
#include "CommandQueueImpl.hpp"
#include "DescriptorSetImpl.hpp"
#include "FenceImpl.hpp"
#include "FramebufferImpl.hpp"
#include "ImageImpl.hpp"
#include "PipelineStateImpl.hpp"
#include "SamplerImpl.hpp"
#include "SemaphoreImpl.hpp"
#include "ShaderModuleImpl.hpp"
#include "SurfaceImpl.hpp"
#include "VulkanFormat.hpp"

#include <array>
#include <cassert>
#include <chrono>
#include <memory>
#include <mutex>


using namespace Coral::Vulkan;

ContextImpl::~ContextImpl()
{
	mDescriptorSetPool.reset();
	mStagingBufferPool.reset();

	mTransferQueue.reset();	 
	mGraphicsQueue.reset();
	mComputeQueue.reset();

	if (mAllocator != VK_NULL_HANDLE)
	{
		vmaDestroyAllocator(mAllocator);
	}

	if (mDevice != VK_NULL_HANDLE)
	{
		vkDestroyDevice(mDevice, nullptr);
	}

	if (mDebugMessenger)
	{
		vkb::destroy_debug_utils_messenger(mInstance, mDebugMessenger, nullptr);
	}

	if (mInstance != VK_NULL_HANDLE)
	{
		vkDestroyInstance(mInstance, nullptr);
	}
}


Coral::ContextPtr
ContextImpl::create(const ContextCreateConfig& config)
{
	auto context = new Coral::Vulkan::ContextImpl;

	if (!context->init(config))
	{
		delete context;
		return nullptr;
	}

	return Coral::ContextPtr{ context };
}


bool
ContextImpl::init(const ContextCreateConfig& config)
{
	if (volkInitialize() != VK_SUCCESS)
	{
		return false;
	}

	// Enable Vulkan Validation Layers only in debug builds
#ifdef NDEBUG
	bool requestValidationLayers{ false };
#else
	bool requestValidationLayers{ true };
#endif

	std::string appName = config.applicationName.empty() ? "Coral" : std::string(config.applicationName);

	vkb::InstanceBuilder builder;
	auto instance = builder
		.set_app_name(appName.c_str())
		.request_validation_layers(requestValidationLayers)
		.use_default_debug_messenger()
		.require_api_version(1, 3, 0)
		.build();

	if (!instance)
	{
		return false;
	}

	mInstance		= instance->instance;
	mDebugMessenger = instance->debug_messenger;
	volkLoadInstance(mInstance);

	// Vulkan 1.0 features
	VkPhysicalDeviceFeatures features10{};
	features10.fillModeNonSolid = VK_TRUE;

	// Vulkan 1.1 features
	VkPhysicalDeviceVulkan11Features features11{};

	// Vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{};
	features12.bufferDeviceAddress = VK_TRUE;
	features12.timelineSemaphore   = VK_TRUE;

	// Vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features13{};
	features13.dynamicRendering = VK_TRUE;
	features13.synchronization2 = VK_TRUE;

	// use vkbootstrap to select a GPU. 
	// We want a GPU that can write to the surface and supports Vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{ instance.value()};
	auto physicalDevice = selector
		.set_minimum_version(1, 3)
		.set_required_features(features10)
		.set_required_features_11(features11)
		.set_required_features_12(features12)
		.set_required_features_13(features13)
		.defer_surface_initialization()
		.add_required_extension(VK_EXT_NESTED_COMMAND_BUFFER_EXTENSION_NAME)
		.select();

	if (!physicalDevice)
	{
		//Log::error("Failed to select physical device: {}", physicalDevice.error().message());
		return false;
	}

	mPhysicalDevice = physicalDevice->physical_device;

	std::optional<uint32_t> queueFamilyIndex;
	// Look for a device queue family that supports GRAPHICS, COMPUTE and 
	// TRANSFER in one, so we don't need command pool for different queue
	// families. Ideally, the queue family also for a dedicated queue for each
	// queue type.
	uint32_t index{ 0 };
	auto queueFamilies = physicalDevice->get_queue_families();
	for (const auto& queueFamily : queueFamilies)
	{
		// We don't need to check for VK_QUEUE_TRANSFER_BIT explicitely.
		// 
		// All commands that are allowed on a queue that supports transfer 
		// operations are also allowed on a queue that supports either graphics 
		// or compute operations.Thus, if the capabilities of a queue family 
		// include VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT, then 
		// reporting the VK_QUEUE_TRANSFER_BIT capability separately for that 
		// queue family is optional.
		auto flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;

		auto isSuitable = (queueFamily.queueFlags & flags) == flags;

		if (!isSuitable)
		{
			continue;
		}

		queueFamilyIndex = index;

		// We found a queue family that supports three or more queues. We
		// consider this as optimal so we can have dedicated GRAPHICS, COMPUTE 
		// and TRANSFER queues.
		if (queueFamily.queueCount > 2)
		{
			break;
		}
		index++;
	}

	if (!queueFamilyIndex)
	{
		return false;
	}

	mQueueFamilyIndex = *queueFamilyIndex;

	auto numDedicatedQueues = std::min(static_cast<uint32_t>(queueFamilies.size()), 3u);

	std::vector<float> priorities(numDedicatedQueues, 1.f);

	vkb::CustomQueueDescription queueDescription{ mQueueFamilyIndex, priorities };

	vkb::DeviceBuilder deviceBuilder{ physicalDevice.value()};
	auto device = deviceBuilder.custom_queue_setup({ queueDescription })
							   .build();

	if (!device)
	{
		return false;
	}

	mDevice = device->device;
	volkLoadDevice(mDevice);

	switch (numDedicatedQueues)
	{
		case 1:
		{
			VkQueue queue{ VK_NULL_HANDLE };
			vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 0, &queue);

			mGraphicsQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(this, queue, 0, mQueueFamilyIndex);
			mComputeQueue	= mGraphicsQueue;
			mTransferQueue	= mGraphicsQueue;
			break;
		}
		case 2:
		{
			VkQueue queue0{ VK_NULL_HANDLE };
			VkQueue queue1{ VK_NULL_HANDLE };
			vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 0, &queue0);
			vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 1, &queue1);

			mGraphicsQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(this, queue0, 0, mQueueFamilyIndex);
			mComputeQueue	= mGraphicsQueue;
			mTransferQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(this, queue1, 1, mQueueFamilyIndex);
			break;
		}
		case 3:
		{
			VkQueue queue0{ VK_NULL_HANDLE };
			VkQueue queue1{ VK_NULL_HANDLE };
			VkQueue queue2{ VK_NULL_HANDLE };
			vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 0, &queue0);
			vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 1, &queue1);
			vkGetDeviceQueue(mDevice, mQueueFamilyIndex, 2, &queue2);

			mGraphicsQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(this, queue0, 0, mQueueFamilyIndex);
			mComputeQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(this, queue1, 1, mQueueFamilyIndex);
			mTransferQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(this, queue2, 2, mQueueFamilyIndex);
			break;
		}
		default:
			assert(false);
			return false;
	}

	VmaAllocatorCreateInfo allocatorCreateInfo{};
	allocatorCreateInfo.device				= mDevice;
	allocatorCreateInfo.instance			= mInstance;
	allocatorCreateInfo.physicalDevice		= mPhysicalDevice;
	allocatorCreateInfo.vulkanApiVersion	= VK_API_VERSION_1_3;

	VmaVulkanFunctions functions{};
	functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	allocatorCreateInfo.pVulkanFunctions = &functions;

	//allocatorCreateInfo.flags				= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	// Allocate 64 MiB sized memory block chunks. If requested, the allocator 
	// will create larger memory blocks to ensure continuous memory per buffer.
	allocatorCreateInfo.preferredLargeHeapBlockSize = 1024 * 1024 * 64;
	if (vmaCreateAllocator(&allocatorCreateInfo, &mAllocator) != VK_SUCCESS)
	{
		return false;
	}

	mStagingBufferPool = std::make_unique<BufferPool>(*this, Coral::BufferType::STORAGE_BUFFER, true);
	mDescriptorSetPool = std::make_unique<DescriptorSetPool>(*this);

	vkGetPhysicalDeviceProperties(mPhysicalDevice, &mProperties);

	return true;
}


Coral::CommandQueue*
ContextImpl::getGraphicsQueue()
{
	return mGraphicsQueue.get();
}


Coral::CommandQueue*
ContextImpl::getTransferQueue()
{
	return mTransferQueue.get();
}


Coral::CommandQueue*
ContextImpl::getComputeQueue()
{
	return mComputeQueue.get();
}


std::expected<Coral::BufferPtr, Coral::BufferCreationError>
ContextImpl::createBuffer(const Coral::BufferCreateConfig& config)
{
	return create<Coral::Buffer, BufferImpl, Coral::BufferCreateConfig, Coral::BufferCreationError>(config);
}


std::expected<Coral::BufferViewPtr, Coral::BufferViewCreationError>
ContextImpl::createBufferView(const Coral::BufferViewCreateConfig& config)
{
	return create<Coral::BufferView, BufferViewImpl, Coral::BufferViewCreateConfig, Coral::BufferViewCreationError>(config);
}


std::expected<Coral::DescriptorSetPtr, Coral::DescriptorSetCreationError>
ContextImpl::createDescriptorSet(const Coral::DescriptorSetCreateConfig& config)
{
	return create<Coral::DescriptorSet, DescriptorSetImpl, Coral::DescriptorSetCreateConfig, Coral::DescriptorSetCreationError>(config);
}


std::expected<Coral::FencePtr, Coral::FenceCreationError>
ContextImpl::createFence()
{
	return create<Coral::Fence, FenceImpl, Coral::FenceCreationError>();
}


std::expected<Coral::FramebufferPtr, Coral::FramebufferCreationError>
ContextImpl::createFramebuffer(const Coral::FramebufferCreateConfig& config)
{
	return create<Coral::Framebuffer, FramebufferImpl, Coral::FramebufferCreateConfig, Coral::FramebufferCreationError>(config);
}


std::expected<Coral::ImagePtr, Coral::ImageCreationError>
ContextImpl::createImage(const Coral::ImageCreateConfig& config)
{
	return create<Coral::Image, ImageImpl, Coral::ImageCreateConfig, Coral::ImageCreationError>(config);
}


std::expected<Coral::PipelineStatePtr, Coral::PipelineStateCreationError>
ContextImpl::createPipelineState(const Coral::PipelineStateCreateConfig& config)
{
	return create<Coral::PipelineState, PipelineStateImpl, Coral::PipelineStateCreateConfig, Coral::PipelineStateCreationError>(config);
}


std::expected<Coral::SamplerPtr, Coral::SamplerCreationError>
ContextImpl::createSampler(const Coral::SamplerCreateConfig& config)
{
	return create<Coral::Sampler, SamplerImpl, Coral::SamplerCreateConfig, Coral::SamplerCreationError>(config);
}


std::expected<Coral::SemaphorePtr, Coral::SemaphoreCreationError>
ContextImpl::createSemaphore()
{
	return create<Coral::Semaphore, SemaphoreImpl, Coral::SemaphoreCreationError>();
}


std::expected<Coral::ShaderModulePtr, Coral::ShaderModuleCreationError>
ContextImpl::createShaderModule(const Coral::ShaderModuleCreateConfig& config)
{
	return create<Coral::ShaderModule, ShaderModuleImpl, Coral::ShaderModuleCreateConfig, Coral::ShaderModuleCreationError>(config);
}


std::expected<Coral::SurfacePtr, Coral::SurfaceCreationError>
ContextImpl::createSurface(const Coral::SurfaceCreateConfig& config)
{
	return create<Coral::Surface, SurfaceImpl, Coral::SurfaceCreateConfig, Coral::SurfaceCreationError>(config);
}


VmaAllocator 
ContextImpl::getVmaAllocator()
{
	return mAllocator;
}


uint32_t 
ContextImpl::getQueueFamilyIndex()
{
	return mQueueFamilyIndex;
}


Coral::BufferPtr
ContextImpl::requestStagingBuffer(size_t bufferSize)
{
	return mStagingBufferPool->requestBuffer(bufferSize);
}


void
ContextImpl::returnStagingBuffers(std::vector<Coral::BufferPtr>&& stagingBuffers)
{
	mStagingBufferPool->returnBuffers(std::forward<std::vector<Coral::BufferPtr>>(stagingBuffers));
}
