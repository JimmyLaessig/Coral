#include "SurfaceImpl.hpp"

#include "FenceImpl.hpp"
#include "FramebufferImpl.hpp"
#include "ImageImpl.hpp"
#include "VulkanFormat.hpp"
#include "SemaphoreImpl.hpp"
#include "CommandQueueImpl.hpp"
#include "CommandBufferImpl.hpp"

#include <algorithm>
#include <memory>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

VkSurfaceFormatKHR
chooseSwapchainFormat(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t formatCount{ 0 };
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());

	for (auto format : formats)
	{
		if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}
	return formats.front();
}

} // namespace

SurfaceImpl::~SurfaceImpl()
{
	if (mSwapchain)
	{
		vkDestroySwapchainKHR(mContext->getVkDevice(), mSwapchain, nullptr);
	}

	if (mSurface)
	{
		vkDestroySurfaceKHR(mContext->getVkInstance(), mSurface, nullptr);
	}
}


bool
SurfaceImpl::initSwapchain(const Coral::SwapchainConfig& config)
{
	mConfig = config;

	mSwapchainImageData.clear();
	mSwapchainDepthImage.reset();
	auto surfaceFormat = chooseSwapchainFormat(mContext->getVkPhysicalDevice(), mSurface);

	VkSurfaceCapabilitiesKHR surfaceCapabilites{ };
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext->getVkPhysicalDevice(), mSurface, &surfaceCapabilites) != VK_SUCCESS)
	{
		return false;
	}

	mSwapchainExtent.width  = std::clamp(surfaceCapabilites.currentExtent.width, surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width);
	mSwapchainExtent.height = std::clamp(surfaceCapabilites.currentExtent.height, surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height);

	VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface			= mSurface;
	createInfo.minImageCount	= config.swapchainImageCount;
	createInfo.imageFormat		= surfaceFormat.format;
	createInfo.imageColorSpace	= surfaceFormat.colorSpace;
	createInfo.imageExtent		= mSwapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.presentMode		= VK_PRESENT_MODE_FIFO_KHR;
	createInfo.oldSwapchain		= mSwapchain;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform		= surfaceCapabilites.currentTransform;
	createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if (vkCreateSwapchainKHR(mContext->getVkDevice(), &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
	{
		return false;
	}

	// Query the number of swapchain images
	if (vkGetSwapchainImagesKHR(mContext->getVkDevice(), mSwapchain, &mSwapchainImageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	// Query the handles to the swapchain images
	std::vector<VkImage> swapchainImages(mSwapchainImageCount);
	if (vkGetSwapchainImagesKHR(mContext->getVkDevice(), mSwapchain, &mSwapchainImageCount, swapchainImages.data()) != VK_SUCCESS)
	{
		return false;
	}

	// Create image views
	mSwapchainImageData.resize(mSwapchainImageCount);
	for (size_t i = 0; i < mSwapchainImageCount; ++i)
	{
		auto image = std::make_unique<Coral::Vulkan::ImageImpl>();

		// Store the swapchain image
		if (!image->init(*mContext,
			swapchainImages[i],
			convert(createInfo.imageFormat),
			mSwapchainExtent.width,
			mSwapchainExtent.height,
			1,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR))
		{
			return false;
		}

		mSwapchainImageData[i].image.reset(image.release());
	}

	// Create the depth buffer
	if (config.depthFormat)
	{
		auto depthFormat = Coral::Vulkan::convert(*config.depthFormat);

		if (!isDepthFormat(*config.depthFormat))
		{
			return false;
		}

		Coral::ImageCreateConfig depthImageConfig{};
		depthImageConfig.format = *config.depthFormat;
		depthImageConfig.width = createInfo.imageExtent.width;
		depthImageConfig.height = createInfo.imageExtent.height;

		auto swapchainImage = mContext->createImage(depthImageConfig);
		if (!swapchainImage)
		{
			return false;
		}

		mSwapchainDepthImage = std::move(swapchainImage.value());
	}

	// Create the Framebuffer for each swapchain image
	for (size_t i = 0; i < mSwapchainImageCount; ++i)
	{

		Coral::FramebufferCreateConfig framebufferConfig{};

		Coral::ColorAttachment colorAttachment{ mSwapchainImageData[i].image.get(), Coral::ClearOp::CLEAR };

		framebufferConfig.colorAttachment = { &colorAttachment, 1 };

		if (config.depthFormat)
		{
			framebufferConfig.depthAttachment = { mSwapchainDepthImage.get(), Coral::ClearOp::CLEAR };
		}

		auto framebuffer = mContext->createFramebuffer(framebufferConfig);

		if (!framebuffer.has_value())
		{
			return false;
		}

		mSwapchainImageData[i].framebuffer = std::move(framebuffer.value());
	}
	
	if (mSwapchainSyncObjects.empty())
	{
		for (size_t i = 0; i < mSwapchainImageCount; ++i)
		{
			SwapchainSyncObjects syncs{};
			syncs.imageReadySemaphore = mContext->createSemaphore().value();
			syncs.presentSemaphore    = mContext->createSemaphore().value();
			syncs.acquireSemaphore    = mContext->createSemaphore().value();

			mSwapchainSyncObjects.push_back(std::move(syncs));
		}
	}

	return true;
}


std::optional<Coral::SurfaceCreationError>
SurfaceImpl::init(Coral::Vulkan::ContextImpl& context, const Coral::SurfaceCreateConfig& config)
{
	mContext = &context;
	mSurface = Coral::Vulkan::createVkSurface(context.getVkInstance(), config.nativeWindowHandle);

	if (mSurface == VK_NULL_HANDLE)
	{
		return Coral::SurfaceCreationError::INTERNAL_ERROR;
	}

	if (!initSwapchain(config.swapchainConfig))
	{
		return Coral::SurfaceCreationError::INTERNAL_ERROR;
	}

	return {};
}


VkSurfaceKHR
SurfaceImpl::getVkSurface()
{
	return mSurface;
}


VkSwapchainKHR
SurfaceImpl::getVkSwapchain()
{
	return mSwapchain;
}


void*
SurfaceImpl::nativeWindowHandle()
{
	return mNativeWindowHandle;
}


Coral::SwapchainImageInfo
SurfaceImpl::acquireNextSwapchainImage(Coral::Fence* fence)
{
	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
	// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
	// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' in order to
	// present the swapchain image. Both layout transitions must be executed manually via memory barriers. 

	auto commandQueue = mContext->getGraphicsQueue();

	auto& syncObjects = mSwapchainSyncObjects[mCurrentSwapchainIndex];
	syncObjects.acquireCommandBuffer = commandQueue->createCommandBuffer({}).value();

	auto acquireSemaphore    = syncObjects.acquireSemaphore.get();
	auto imageReadySemaphore = syncObjects.imageReadySemaphore.get();
	auto commandBuffer		 = syncObjects.acquireCommandBuffer.get();

	auto acquireSemaphoreImpl    = static_cast<SemaphoreImpl*>(acquireSemaphore); 
	auto imageReadySemaphoreImpl = static_cast<SemaphoreImpl*>(imageReadySemaphore);
	auto commandBufferImpl       = static_cast<CommandBufferImpl*>(commandBuffer);

	auto vkFence = fence ? static_cast<Coral::Vulkan::FenceImpl*>(fence)->getVkFence() : VK_NULL_HANDLE;
	auto result = vkAcquireNextImageKHR(mContext->getVkDevice(), mSwapchain, UINT32_MAX, acquireSemaphoreImpl->getVkSemaphore(), vkFence, &mCurrentSwapchainIndex);

	// check if VK_ERROR_OUT_OF_DATE_KHR, then rebuild the swapchain
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkDeviceWaitIdle(mContext->getVkDevice());

		if (!initSwapchain(mConfig))
		{
			return {};
		}

		return acquireNextSwapchainImage(fence);
	}

	if (result != VK_SUCCESS)
	{
		return {};
	}

	mCurrentSwapchainImageInfo                         = {};
	mCurrentSwapchainImageInfo.imageAvailableSemaphore = imageReadySemaphore;
	mCurrentSwapchainImageInfo.image                   = mSwapchainImageData[mCurrentSwapchainIndex].image.get();
	mCurrentSwapchainImageInfo.depthImage              = mSwapchainDepthImage.get();
	mCurrentSwapchainImageInfo.framebuffer             = mSwapchainImageData[mCurrentSwapchainIndex].framebuffer.get();

	commandBuffer->begin();

	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.image                           = static_cast<ImageImpl*>(mCurrentSwapchainImageInfo.image)->getVkImage();
	barrier.srcQueueFamilyIndex             = mContext->getQueueFamilyIndex();
	barrier.dstQueueFamilyIndex             = mContext->getQueueFamilyIndex();
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = mCurrentSwapchainImageInfo.image->getMipLevels();

	vkCmdPipelineBarrier(commandBufferImpl->getVkCommandBuffer(),
	                     VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,             // srcStageMask
	                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
	                     0,
	                     0,
	                     nullptr,
	                     0,
	                     nullptr,
	                     1,
	                     &barrier);

	commandBuffer->end();

	Coral::CommandBufferSubmitInfo info{};
	info.commandBuffers   = { &commandBuffer, 1 };
	info.waitSemaphores   = { &acquireSemaphore, 1 };
	info.signalSemaphores = { &imageReadySemaphore, 1 };
	commandQueue->submit(info, nullptr);

	return mCurrentSwapchainImageInfo;
}


Coral::SwapchainImageInfo
SurfaceImpl::getCurrentSwapchainImage()
{
	return mCurrentSwapchainImageInfo;
}


 uint32_t
SurfaceImpl::getCurrentSwapchainImageIndex()
{
	 return mCurrentSwapchainIndex;
}


uint32_t
SurfaceImpl::getSwapchainImageCount() const
{
	return static_cast<uint32_t>(mSwapchainImageData.size());
}


Coral::FramebufferSignature
SurfaceImpl::getFramebufferSignature()
{
	return mSwapchainImageData.front().framebuffer->getSignature();
}


void
SurfaceImpl::present(CommandQueueImpl& commandQueue, std::span<Semaphore*> waitSemaphores)
{
	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
	// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
	// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' before present the
	// swapchain image. Both layout transitions must be executed manually via memory barriers.

	auto& syncObjects = mSwapchainSyncObjects[mCurrentSwapchainIndex];
	syncObjects.presentCommandBuffer = commandQueue.createCommandBuffer({}).value();

	auto commandBuffer       = syncObjects.presentCommandBuffer.get();
	auto presentSemaphore    = syncObjects.presentSemaphore.get();
	auto swapchainImage      = static_cast<ImageImpl*>(mCurrentSwapchainImageInfo.image);
	auto swapchainImageIndex = mCurrentSwapchainIndex;

	commandBuffer->begin();
	
	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.srcAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.image                           = swapchainImage->getVkImage();
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = swapchainImage->getMipLevels();

	vkCmdPipelineBarrier(static_cast<CommandBufferImpl*>(commandBuffer)->getVkCommandBuffer(),
		                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
		                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,          // dstStageMask
		                 0,
		                 0,
		                 nullptr,
		                 0,
		                 nullptr,
		                 1, // imageMemoryBarrierCount
		                 &barrier);

	commandBuffer->end();

	Coral::CommandBufferSubmitInfo info{};
	info.waitSemaphores   = waitSemaphores;
	info.commandBuffers   = { &commandBuffer, 1 };
	info.signalSemaphores = { &presentSemaphore, 1 };

	commandQueue.submit(info, nullptr);

	VkSemaphore vkSemaphore = static_cast<SemaphoreImpl*>(presentSemaphore)->getVkSemaphore();

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pWaitSemaphores    = &vkSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices      = &swapchainImageIndex;
	presentInfo.pSwapchains        = &mSwapchain;
	presentInfo.swapchainCount     = 1;
	
	vkQueuePresentKHR(commandQueue.getVkQueue(), &presentInfo);
}
