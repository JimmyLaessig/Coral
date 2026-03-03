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
		vkDestroySwapchainKHR(context().getVkDevice(), mSwapchain, nullptr);
	}

	if (mSurface)
	{
		vkDestroySurfaceKHR(context().getVkInstance(), mSurface, nullptr);
	}
}


bool
SwapchainImpl::initSwapchain(const Coral::Swapchain::CreateConfig& config)
{
	mConfig = config;

	mImageAcquiredSemaphore.clear();
	mTransitionToColorAttachment.clear();
	mImageReadySemaphore.clear();
	mTransitionToPresent.clear();
	mImagePresentableSemaphore.clear();
	mSwapchainImages.clear();
	mFramebuffers.clear();
	mSwapchainDepthImage.reset();

	auto surfaceFormat = chooseSwapchainFormat(context().getVkPhysicalDevice(), mSurface, convert(config.format));

	if (!surfaceFormat)
	{
		return false;
	}

	mSurfaceFormat = *surfaceFormat;

	VkSurfaceCapabilitiesKHR surfaceCapabilites{ };
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context().getVkPhysicalDevice(), mSurface, &surfaceCapabilites) != VK_SUCCESS)
	{
		return false;
	}

	mSwapchainExtent.width  = std::clamp(surfaceCapabilites.currentExtent.width, surfaceCapabilites.minImageExtent.width, surfaceCapabilites.maxImageExtent.width);
	mSwapchainExtent.height = std::clamp(surfaceCapabilites.currentExtent.height, surfaceCapabilites.minImageExtent.height, surfaceCapabilites.maxImageExtent.height);

	VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface			= mSurface;
	createInfo.minImageCount	= config.minImageCount;
	createInfo.imageFormat		= mSurfaceFormat.format;
	createInfo.imageColorSpace	= mSurfaceFormat.colorSpace;
	createInfo.imageExtent		= mSwapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.presentMode      = choosePresentMode(context().getVkPhysicalDevice(), mSurface, config.lockToVSync);
	createInfo.oldSwapchain		= mSwapchain;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform		= surfaceCapabilites.currentTransform;
	createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if (vkCreateSwapchainKHR(context().getVkDevice(), &createInfo, nullptr, &mSwapchain) != VK_SUCCESS)
	{
		return false;
	}

	// Destroy the old swapchain after creating the new swapchain
	if (createInfo.oldSwapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(context().getVkDevice(), createInfo.oldSwapchain, nullptr);
	}

	// Query the number of swapchain images
	if (vkGetSwapchainImagesKHR(context().getVkDevice(), mSwapchain, &mSwapchainImageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	// Query the handles to the swapchain images
	std::vector<VkImage> swapchainImages(mSwapchainImageCount);
	if (vkGetSwapchainImagesKHR(context().getVkDevice(), mSwapchain, &mSwapchainImageCount, swapchainImages.data()) != VK_SUCCESS)
	{
		return false;
	}


	// Create the depth buffer
	if (config.depthFormat)
	{
		auto depthFormat = Coral::Vulkan::convert(*config.depthFormat);

		if (!isDepthFormat(*config.depthFormat))
		{
			return false;
		}

		Coral::Image::CreateConfig depthImageConfig{};
		depthImageConfig.format        = *config.depthFormat;
		depthImageConfig.extent.width  = createInfo.imageExtent.width;
		depthImageConfig.extent.height = createInfo.imageExtent.height;

		auto swapchainImage = context().createImage(depthImageConfig);
		if (!swapchainImage)
		{
			return false;
		}

		mSwapchainDepthImage = swapchainImage.value();
	}


	for (size_t i = 0; i < mSwapchainImageCount; ++i)
	{
		// Create swapchain images
		auto image = std::make_shared<Coral::Vulkan::ImageImpl>(context());
		if (!image->init(swapchainImages[i],
			             convert(createInfo.imageFormat),
			             mSwapchainExtent.width,
			             mSwapchainExtent.height,
			             1,
						 CO_IMAGE_USAGE_HINT_FRAMEBUFFER_ATTACHMENT))
		{
			return false;
		}

		mSwapchainImages.push_back(image);

		// Create the framebuffer 
		Coral::ColorAttachment colorAttachment;

		Coral::Framebuffer::CreateConfig framebufferConfig{};
		framebufferConfig.colorAttachments = { { mSwapchainImages[i], 0 } };
		framebufferConfig.depthAttachment  = mSwapchainDepthImage;

		auto framebuffer = context().createFramebuffer(framebufferConfig);
		if (!framebuffer.has_value())
		{
			return false;
		}

		mFramebuffers.push_back(framebuffer.value());

		// Create the imageAcquiredSemaphore
		mImageAcquiredSemaphore.push_back(context().createSemaphore({}).value());
		// Create the imageReadySemaphore
		mImageReadySemaphore.push_back(context().createSemaphore({}).value());
		// Create the mImagePresentableSemaphore
		mImagePresentableSemaphore.push_back(context().createSemaphore({}).value());
	}

	mTransitionToColorAttachment.resize(mSwapchainImageCount);
	mTransitionToPresent.resize(mSwapchainImageCount);

	return true;
}


std::optional<Coral::Swapchain::CreateError>
SwapchainImpl::init(const Coral::Swapchain::CreateConfig& config)
{
	mSurface = Coral::Vulkan::createVkSurface(context().getVkInstance(), config.nativeWindowHandle);

	if (mSurface == VK_NULL_HANDLE)
	{
		return Coral::Swapchain::CreateError::INTERNAL_ERROR;
	}

	if (!initSwapchain(config))
	{
		return Coral::Swapchain::CreateError::INTERNAL_ERROR;
	}

	return {};
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
SwapchainImpl::acquireNextSwapchainImage(Coral::FencePtr fence)
{
	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
	// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
	// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' in order to
	// present the swapchain image. Both layout transitions must be executed manually via memory barriers. 

	auto commandQueue = context().getGraphicsQueue();

	auto imageAcquiredSemaphore     = mImageAcquiredSemaphore[mCurrentSwapchainIndex];
	auto imageAcquiredSemaphoreImpl = std::static_pointer_cast<SemaphoreImpl>(imageAcquiredSemaphore);

	// Acquire the next swapchain image and signal the `imageAcquiredSemaphore` semaphore that the layout can be
	// transitioned to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
	auto result = vkAcquireNextImageKHR(context().getVkDevice(),
		                                mSwapchain,
		                                UINT32_MAX,
		                                imageAcquiredSemaphoreImpl->getVkSemaphore(),
		                                VK_NULL_HANDLE,
		                                &mCurrentSwapchainIndex);

	// If the swapchain is out of date we have to rebuild the swapchain. For that, we wait for the device to be idle,
	// recreate the swapchain and try image acquisition again
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		vkDeviceWaitIdle(context().getVkDevice());
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

	// Transition the image layout to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	auto commandBuffer     = commandQueue->createCommandBuffer({}).value();
	auto commandBufferImpl = std::static_pointer_cast<CommandBufferImpl>(commandBuffer);

	// We store the command buffer for each swapchain image but recreate a new one each frame (destroyed the old one).
	// This way, the command buffer's lifetime is guaranteed to exceed the duration of this function and the command
	// buffer was created from the correct command queue.
	mTransitionToColorAttachment[mCurrentSwapchainIndex] = commandBuffer;

	auto imageReadySemaphore     = mImageReadySemaphore[mCurrentSwapchainIndex];
	auto imageReadySemaphoreImpl = std::static_pointer_cast<SemaphoreImpl>(imageReadySemaphore);

	commandBuffer->begin();

	auto image = std::static_pointer_cast<ImageImpl>(mSwapchainImages[mCurrentSwapchainIndex]);
	ImageImpl::cmdTransitionImageLayout(commandBufferImpl->getVkCommandBuffer(), 
                                        *image, 
                                        image->getPreferredImageLayout(), 
                                        /*firstMipLevel*/ 0, 
                                        /*levelCount*/ 1, 
                                        /*srcAccessMask*/ VK_ACCESS_NONE, 
                                        /*dstAccessMask*/ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 
                                        /*srcStageFlags*/ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                                        /*dstStageFlags*/ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	commandBuffer->end();

	// Finally, submit the command buffer to transition the image layout to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// for rendering. This command buffer must await the `imageAcquiredSemaphore` for proper synchronization. Also,
	// this command buffer signals the public-facing `imageReadySemaphore` of the current swapchain image so user can
	// properly schedule render commands to only execute once the new swapchain image is ready for rendering.

	Coral::CommandBufferSubmitInfo info{};
	info.commandBuffers   = { commandBuffer };
	info.waitSemaphores   = { imageAcquiredSemaphore };
	info.signalSemaphores = { imageReadySemaphore };
	commandQueue->submit(info, fence);

	return {
		mSwapchainImages[mCurrentSwapchainIndex],
		mFramebuffers[mCurrentSwapchainIndex],
		mImageReadySemaphore[mCurrentSwapchainIndex],
	};
}


void
SwapchainImpl::present(CommandQueueImpl& commandQueue, const std::vector<SemaphorePtr>& waitSemaphores)
{
	std::lock_guard lock(mThreadProtection);
	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
	// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
	// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' before present the
	// swapchain image. Both layout transitions must be executed manually via memory barriers.

	auto commandBuffer                 = commandQueue.createCommandBuffer({}).value(); 
	auto commandBufferImpl             = std::static_pointer_cast<CommandBufferImpl>(commandBuffer);
	auto imagePresentableSemaphore     = mImagePresentableSemaphore[mCurrentSwapchainIndex];
	auto imagePresentableSemaphoreImpl = std::static_pointer_cast<SemaphoreImpl>(imagePresentableSemaphore);
	auto swapchainImageImpl            = std::static_pointer_cast<ImageImpl>(mSwapchainImages[mCurrentSwapchainIndex]);

	// We store the command buffer for each swapchain image but recreate a new one each frame (destroyed the old one).
	// This way, the command buffer's lifetime is guaranteed to exceed the duration of this function and the command
	// buffer was created from the correct command queue.
	mTransitionToPresent[mCurrentSwapchainIndex] = commandBuffer;

	// Use a built-in command buffer to perform memory layout transition from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR prior to presenting the image.
	commandBuffer->begin();

	ImageImpl::cmdTransitionImageLayout(commandBufferImpl->getVkCommandBuffer(),
		                                *swapchainImageImpl,
		                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		                                /*firstMipLevel*/ 0,
		                                /*levelCount*/ 1,
		                                /*srcAccessMask*/ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		                                /*dstAccessMask*/ VK_ACCESS_NONE,
		                                /*srcStageFlags*/ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		                                /*dstStageFlags*/ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

	commandBuffer->end();

	// Submit the command buffer from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR. Use
	// the `presentSemaphore` to signal the vkQueuePresentKHR call when the layout transition has finished. Note that
	// this command buffer also awaits the suppied `waitSemaphores` to ensure proper synchronization.
	Coral::CommandBufferSubmitInfo info{};
	info.waitSemaphores   = waitSemaphores;
	info.commandBuffers   = { commandBuffer };
	info.signalSemaphores = { imagePresentableSemaphore };

	commandQueue.submit(info, nullptr);

	// Finally, dispatch the vkQueuePresentKHR call to display the newly rendered image. Note that this call must wait
	// for the presentSemaphore so that presentation is performed after the memory layout transition is done.
	VkSemaphore vkSemaphore = imagePresentableSemaphoreImpl->getVkSemaphore();

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.pWaitSemaphores    = &vkSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices      = &mCurrentSwapchainIndex;
	presentInfo.pSwapchains        = &mSwapchain;
	presentInfo.swapchainCount     = 1;

	vkQueuePresentKHR(commandQueue.getVkQueue(), &presentInfo);
}


Coral::SwapchainImageInfo
SwapchainImpl::currentSwapchainImage() const
{
	std::lock_guard lock(mThreadProtection);
	return
	{
		mSwapchainImages[mCurrentSwapchainIndex],
		mFramebuffers[mCurrentSwapchainIndex],
		mImageReadySemaphore[mCurrentSwapchainIndex],
	};
}


uint32_t
SwapchainImpl::currentSwapchainImageIndex() const
{
	std::lock_guard lock(mThreadProtection);
	return mCurrentSwapchainIndex;
}


uint32_t
SwapchainImpl::swapchainImageCount() const
{
	std::lock_guard lock(mThreadProtection);
	return static_cast<uint32_t>(mSwapchainImages.size());
}


CoExtent
SwapchainImpl::swapchainExtent() const
{
	std::lock_guard lock(mThreadProtection);
	return { mSwapchainImages.front()->width(), mSwapchainImages.front()->height()};
}


Coral::Framebuffer::Layout
SwapchainImpl::framebufferLayout() const
{
	std::lock_guard lock(mThreadProtection);
	return mFramebuffers.front()->layout();
}
