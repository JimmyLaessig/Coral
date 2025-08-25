#include <Coral/Vulkan/SwapchainImpl.hpp>

#include <Coral/Vulkan/CommandBufferImpl.hpp>
#include <Coral/Vulkan/CommandQueueImpl.hpp>
#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/FenceImpl.hpp>
#include <Coral/Vulkan/FramebufferImpl.hpp>
#include <Coral/Vulkan/ImageImpl.hpp>
#include <Coral/Vulkan/SemaphoreImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>

#include <algorithm>
#include <memory>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

std::optional<VkSurfaceFormatKHR>
chooseSwapchainFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkFormat requestedFormat)
{
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());

	for (auto format : formats)
	{
		if (format.format == requestedFormat)
		{
			return format;
		}
	}

	return {};
}


VkPresentModeKHR
choosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface, bool lockToVSync)
{
	uint32_t presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModesCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, presentModes.data());

	VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };

	bool mailboxAvailable{ false };
	bool fifoAvailable{ false };
	bool immediateAvailable{ false };

	for (const auto& presentMode : presentModes)
	{
		mailboxAvailable   |= presentMode == VK_PRESENT_MODE_MAILBOX_KHR;
		fifoAvailable      |= presentMode == VK_PRESENT_MODE_FIFO_KHR;
		immediateAvailable |= presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	// Prefer immediate present mode if available and v-sync is disabled. Otherwise choose mailbox or FIFO as last
	// resort.
	if (immediateAvailable && !lockToVSync)
	{
		presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	else if (mailboxAvailable)
	{
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	else if (fifoAvailable)
	{
		presentMode = VK_PRESENT_MODE_FIFO_KHR;
	}
	else
	{
		assert(false);
	}

	return presentMode;
}

} // namespace


SwapchainImpl::~SwapchainImpl()
{
	if (mSwapchain)
	{
		vkDestroySwapchainKHR(contextImpl().getVkDevice(), mSwapchain, nullptr);
	}

	if (mSurface)
	{
		vkDestroySurfaceKHR(contextImpl().getVkInstance(), mSurface, nullptr);
	}
}


bool
SwapchainImpl::initSwapchain(const Coral::SwapchainCreateConfig& config)
{
	mConfig = config;

	mSwapchainImageData.clear();
	mSwapchainDepthImage.reset();

	auto surfaceFormat = chooseSwapchainFormat(contextImpl().getVkPhysicalDevice(), mSurface, convert(config.format));

	if (!surfaceFormat)
	{
		return false;
	}

	mSurfaceFormat = *surfaceFormat;

	VkSurfaceCapabilitiesKHR surfaceCapabilites{ };
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(contextImpl().getVkPhysicalDevice(), mSurface, &surfaceCapabilites) != VK_SUCCESS)
	{
		return false;
	}

	mSwapchainExtent.width  = std::clamp(surfaceCapabilites.currentExtent.width, surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width);
	mSwapchainExtent.height = std::clamp(surfaceCapabilites.currentExtent.height, surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height);

	VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface			= mSurface;
	createInfo.minImageCount	= config.swapchainImageCount;
	createInfo.imageFormat		= mSurfaceFormat.format;
	createInfo.imageColorSpace	= mSurfaceFormat.colorSpace;
	createInfo.imageExtent		= mSwapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.presentMode      = choosePresentMode(contextImpl().getVkPhysicalDevice(), mSurface, config.lockToVSync);
	createInfo.oldSwapchain		= mSwapchain;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform		= surfaceCapabilites.currentTransform;
	createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if (vkCreateSwapchainKHR(contextImpl().getVkDevice(), &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
	{
		return false;
	}

	// Destroy the old swapchain after creating the new swapchain
	if (createInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(contextImpl().getVkDevice(), createInfo.oldSwapchain, nullptr);
	}

	// Query the number of swapchain images
	if (vkGetSwapchainImagesKHR(contextImpl().getVkDevice(), mSwapchain, &mSwapchainImageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	// Query the handles to the swapchain images
	std::vector<VkImage> swapchainImages(mSwapchainImageCount);
	if (vkGetSwapchainImagesKHR(contextImpl().getVkDevice(), mSwapchain, &mSwapchainImageCount, swapchainImages.data()) != VK_SUCCESS)
	{
		return false;
	}

	// Create image views
	mSwapchainImageData.resize(mSwapchainImageCount);
	for (size_t i = 0; i < mSwapchainImageCount; ++i)
	{
		auto image = std::make_unique<Coral::Vulkan::ImageImpl>(contextImpl());

		// Store the swapchain image
		if (!image->init(swapchainImages[i],
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

		auto swapchainImage = contextImpl().createImage(depthImageConfig);
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

		Coral::ColorAttachment colorAttachment{ mSwapchainImageData[i].image.get() };

		framebufferConfig.colorAttachment = { &colorAttachment, 1 };

		if (config.depthFormat)
		{
			framebufferConfig.depthAttachment = { mSwapchainDepthImage.get() };
		}

		auto framebuffer = contextImpl().createFramebuffer(framebufferConfig);

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
			syncs.imageReadySemaphore = contextImpl().createSemaphore().value();
			syncs.imagePresentableSemaphore    = contextImpl().createSemaphore().value();
			syncs.imageAcquiredSemaphore    = contextImpl().createSemaphore().value();

			mSwapchainSyncObjects.push_back(std::move(syncs));
		}
	}

	return true;
}


std::optional<Coral::SwapchainCreationError>
SwapchainImpl::init(const Coral::SwapchainCreateConfig& config)
{
	mSurface = Coral::Vulkan::createVkSurface(contextImpl().getVkInstance(), config.nativeWindowHandle);

	if (mSurface == VK_NULL_HANDLE)
	{
		return Coral::SwapchainCreationError::INTERNAL_ERROR;
	}

	if (!initSwapchain(config))
	{
		return Coral::SwapchainCreationError::INTERNAL_ERROR;
	}

	return {};
}


ContextImpl&
SwapchainImpl::contextImpl()
{
	return static_cast<ContextImpl&>(context());
}


VkSurfaceKHR
SwapchainImpl::getVkSurface()
{
	return mSurface;
}


VkSwapchainKHR
SwapchainImpl::getVkSwapchain()
{
	return mSwapchain;
}


void*
SwapchainImpl::nativeWindowHandle()
{
	return mNativeWindowHandle;
}


Coral::SwapchainImageInfo
SwapchainImpl::acquireNextSwapchainImage(Coral::Fence* fence)
{
	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
	// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
	// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' in order to
	// present the swapchain image. Both layout transitions must be executed manually via memory barriers. 

	auto commandQueue = contextImpl().getGraphicsQueue();

	auto& syncObjects = mSwapchainSyncObjects[mCurrentSwapchainIndex];

	// We store the command buffer for each swapchain image but recreate a new one each frame (destroyed the old one).
	// This way, the command buffer's lifetime is guaranteed to exceed the duration of this function and the command
	// buffer was created from the correct command queue.
	syncObjects.transitionToColorAttachment = commandQueue->createCommandBuffer({}).value();

	auto imageAcquiredSemaphore = syncObjects.imageAcquiredSemaphore.get();
	auto commandBuffer          = syncObjects.transitionToColorAttachment.get();
	auto imageReadySemaphore    = syncObjects.imageReadySemaphore.get();

	auto imageAcquiredSemaphoreImpl = static_cast<SemaphoreImpl*>(imageAcquiredSemaphore);
	auto imageReadySemaphoreImpl    = static_cast<SemaphoreImpl*>(imageReadySemaphore);
	auto commandBufferImpl          = static_cast<CommandBufferImpl*>(commandBuffer);

	// Acquire the next swapchain image and signal the `acquireSemaphore` semaphore that the layout can be
	// transitioned to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
	auto vkFence = fence ? static_cast<Coral::Vulkan::FenceImpl*>(fence)->getVkFence() : VK_NULL_HANDLE;
	auto result = vkAcquireNextImageKHR(contextImpl().getVkDevice(), 
										mSwapchain, 
										UINT32_MAX, 
		                                imageAcquiredSemaphoreImpl->getVkSemaphore(),
										vkFence, 
										&mCurrentSwapchainIndex);

	// If the swapchain is out of date we have to rebuild the swapchain. For that, we wait for the device to be idle,
	// recreate the swapchain and try image acquisition again
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkDeviceWaitIdle(contextImpl().getVkDevice());

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

	// Update the cached SwapchainImageInfo 
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
	barrier.srcQueueFamilyIndex             = contextImpl().getQueueFamilyIndex();
	barrier.dstQueueFamilyIndex             = contextImpl().getQueueFamilyIndex();
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

	// Finally, submit the command buffer to transition the image layout to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// for rendering. This command buffer must await the `imageAcquiredSemaphore` for proper synchronization. Also,
	// this command buffer signals the public-facing `imageReadySemaphore` of the current swapchain image so user can
	// properly schedule render commands to only execute once the new swapchain image is ready for rendering.
	Coral::CommandBufferSubmitInfo info{};
	info.commandBuffers   = { &commandBuffer, 1 };
	info.waitSemaphores   = { &imageAcquiredSemaphore, 1 };
	info.signalSemaphores = { &imageReadySemaphore, 1 };
	commandQueue->submit(info, nullptr);

	return mCurrentSwapchainImageInfo;
}


Coral::SwapchainImageInfo
SwapchainImpl::getCurrentSwapchainImage()
{
	return mCurrentSwapchainImageInfo;
}


 uint32_t
SwapchainImpl::getCurrentSwapchainImageIndex()
{
	 return mCurrentSwapchainIndex;
}


uint32_t
SwapchainImpl::getSwapchainImageCount() const
{
	return static_cast<uint32_t>(mSwapchainImageData.size());
}


Coral::FramebufferSignature
SwapchainImpl::getFramebufferSignature()
{
	return mSwapchainImageData.front().framebuffer->getSignature();
}


void
SwapchainImpl::present(CommandQueueImpl& commandQueue, std::span<Semaphore*> waitSemaphores)
{
	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
	// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
	// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' before present the
	// swapchain image. Both layout transitions must be executed manually via memory barriers.

	auto& syncObjects = mSwapchainSyncObjects[mCurrentSwapchainIndex];

	// We store the command buffer for each swapchain image but recreate a new one each frame (destroyed the old one).
	// This way, the command buffer's lifetime is guaranteed to exceed the duration of this function and the command
	// buffer was created from the correct command queue.
	syncObjects.transitionToPresent = commandQueue.createCommandBuffer({}).value();

	auto commandBuffer             = syncObjects.transitionToPresent.get();
	auto imagePresentableSemaphore = syncObjects.imagePresentableSemaphore.get();
	auto swapchainImage            = static_cast<ImageImpl*>(mCurrentSwapchainImageInfo.image);
	auto swapchainImageIndex       = mCurrentSwapchainIndex;

	// Use a built-in command buffer to perform memory layout transition from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR prior to presenting the image.
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

	// Submit the command buffer from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. Use
	// the `presentSemaphore` to signal the vkQueuePresentKHR call when the layout transition has finished. Note that
	// this command buffer also awaits the suppied `waitSemaphores` to ensure proper synchronization.
	Coral::CommandBufferSubmitInfo info{};
	info.waitSemaphores   = waitSemaphores;
	info.commandBuffers   = { &commandBuffer, 1 };
	info.signalSemaphores = { &imagePresentableSemaphore, 1 };

	commandQueue.submit(info, nullptr);

	// Finally, dispatch the vkQueuePresentKHR call to display the newly rendered image. Note that this call must wait
	// for the presentSemaphore so that presentation is performed after the memory layout transition is done.
	VkSemaphore vkSemaphore = static_cast<SemaphoreImpl*>(imagePresentableSemaphore)->getVkSemaphore();

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pWaitSemaphores    = &vkSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices      = &swapchainImageIndex;
	presentInfo.pSwapchains        = &mSwapchain;
	presentInfo.swapchainCount     = 1;

	vkQueuePresentKHR(commandQueue.getVkQueue(), &presentInfo);
}
