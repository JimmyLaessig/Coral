#include <Coral/Vulkan/ContextImpl.hpp>

#include <Coral/BufferPool.hpp>
#include <Coral/Vulkan/CommandQueueImpl.hpp>
#include <Coral/Vulkan/BufferImpl.hpp>
#include <Coral/Vulkan/FenceImpl.hpp>
#include <Coral/Vulkan/FramebufferImpl.hpp>
#include <Coral/Vulkan/ImageImpl.hpp>
#include <Coral/Vulkan/PipelineStateImpl.hpp>
#include <Coral/Vulkan/SamplerImpl.hpp>
#include <Coral/Vulkan/SemaphoreImpl.hpp>
#include <Coral/Vulkan/ShaderModuleImpl.hpp>
#include <Coral/Vulkan/SwapchainImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>

#include <array>
#include <cassert>
#include <chrono>
#include <memory>
#include <mutex>


using namespace Coral::Vulkan;

ContextImpl::~ContextImpl()
{
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


std::shared_ptr<ContextImpl>
ContextImpl::create(const Context::CreateConfig& config)
{
	auto context = std::make_shared<ContextImpl>();

	if (!context->init(config))
	{
		return nullptr;
	}

	return context;
}


bool
ContextImpl::init(const Context::CreateConfig& config)
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

	std::string appName = config.pApplicationName ? config.pApplicationName : "Coral";

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
		.add_required_extension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME)
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

			mGraphicsQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(*this, queue, 0, mQueueFamilyIndex);
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

			mGraphicsQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(*this, queue0, 0, mQueueFamilyIndex);
			mComputeQueue	= mGraphicsQueue;
			mTransferQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(*this, queue1, 1, mQueueFamilyIndex);
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

			mGraphicsQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(*this, queue0, 0, mQueueFamilyIndex);
			mComputeQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(*this, queue1, 1, mQueueFamilyIndex);
			mTransferQueue	= std::make_shared<Coral::Vulkan::CommandQueueImpl>(*this, queue2, 2, mQueueFamilyIndex);
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

	mStagingBufferPool = std::make_unique<BufferPool>(*this, CO_BUFFER_TYPE_STORAGE, true);

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


std::expected<Coral::BufferPtr, Coral::Buffer::CreateError>
ContextImpl::createBuffer(const Coral::Buffer::CreateConfig& config)
{
	return create<Coral::Buffer, BufferImpl, Coral::Buffer::CreateError>(config);
}


std::expected<Coral::FencePtr, Coral::Fence::CreateError>
ContextImpl::createFence(const Coral::Fence::CreateConfig& config)
{
	return create<Coral::Fence, FenceImpl, Coral::Fence::CreateError>();
}


std::expected<Coral::FramebufferPtr, Coral::Framebuffer::CreateError>
ContextImpl::createFramebuffer(const Coral::Framebuffer::CreateConfig& config)
{
	return create<Coral::Framebuffer, FramebufferImpl, Coral::Framebuffer::CreateError>(config);
}


std::expected<Coral::ImagePtr, Coral::Image::CreateError>
ContextImpl::createImage(const Coral::Image::CreateConfig& config)
{
	return create<Coral::Image, ImageImpl, Coral::Image::CreateError>(config);
}


std::expected<Coral::PipelineStatePtr, Coral::PipelineState::CreateError>
ContextImpl::createPipelineState(const Coral::PipelineState::CreateConfig& config)
{
	return create<Coral::PipelineState, PipelineStateImpl, Coral::PipelineState::CreateError>(config);
}


std::expected<Coral::SamplerPtr, Coral::Sampler::CreateError>
ContextImpl::createSampler(const Coral::Sampler::CreateConfig& config)
{
	return create<Coral::Sampler, SamplerImpl, Coral::Sampler::CreateError>(config);
}


std::expected<Coral::SemaphorePtr, Coral::Semaphore::CreateError>
ContextImpl::createSemaphore(const Coral::Semaphore::CreateConfig& config)
{
	return create<Coral::Semaphore, SemaphoreImpl, Coral::Semaphore::CreateError>();
}


std::expected<Coral::ShaderModulePtr, Coral::ShaderModule::CreateError>
ContextImpl::createShaderModule(const Coral::ShaderModule::CreateConfig& config)
{
	return create<Coral::ShaderModule, ShaderModuleImpl, Coral::ShaderModule::CreateError>(config);
}


std::expected<Coral::SwapchainPtr, Coral::Swapchain::CreateError>
ContextImpl::createSwapchain(const Coral::Swapchain::CreateConfig& config)
{
	return create<Coral::Swapchain, SwapchainImpl,Coral::Swapchain::CreateError>(config);
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


BufferImplPtr
ContextImpl::requestStagingBuffer(size_t bufferSize)
{
	return std::static_pointer_cast<BufferImpl>(mStagingBufferPool->requestBuffer(bufferSize));
}
