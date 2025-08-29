#ifndef CORAL_VULKAN_SWAPCHAINIMPL_HPP
#define CORAL_VULKAN_SWAPCHAINIMPL_HPP

#include <Coral/Swapchain.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

#include <array>
#include <memory>


namespace Coral::Vulkan
{

class SwapchainImpl : public Coral::Swapchain
	                , public Resource
	                , public std::enable_shared_from_this<SwapchainImpl>
{
public:

	using Resource::Resource;

	virtual ~SwapchainImpl();

	std::optional<Coral::SwapchainCreationError> init(const Coral::SwapchainCreateConfig& config);

	void* nativeWindowHandle() override;

	SwapchainImageInfo acquireNextSwapchainImage(Fence* fence) override;

	SwapchainImageInfo getCurrentSwapchainImage() override;

	uint32_t getCurrentSwapchainImageIndex() override;

	uint32_t getSwapchainImageCount() const override;

	FramebufferSignature getFramebufferSignature() override;

	VkSurfaceKHR getVkSurface();

	VkSwapchainKHR getVkSwapchain();

	const Coral::SwapchainCreateConfig& config() const { return mConfig; };

	void present(CommandQueueImpl& commandQueue, std::span<Semaphore*> waitSemaphores);

private:

	bool initSwapchain(const Coral::SwapchainCreateConfig& config);

	void* mNativeWindowHandle{ nullptr };

	VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

	VkSwapchainKHR mSwapchain{ VK_NULL_HANDLE };

	VkSurfaceFormatKHR mSurfaceFormat{};

	uint32_t mSwapchainImageCount{ 0 };

	uint32_t mCurrentSwapchainIndex{ 0 };

	VkExtent2D mSwapchainExtent{ 0, 0 };

	struct SwapchainImageData
	{
		Coral::ImagePtr image;
		Coral::FramebufferPtr framebuffer;
	};

	struct SwapchainSyncObjects
	{
		/// Semaphore to be signaled once the tswapchainhe image has been acquired
		Coral::SemaphorePtr imageAcquiredSemaphore;
		
		/// Commandbuffer to perform layout transition to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
		Coral::CommandBufferPtr transitionToColorAttachment;

		/// Semaphore to be signaled once the swapchain image has been acquired and it's memory layout is transitioned
		/// to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
		Coral::SemaphorePtr imageReadySemaphore;

		/// Commandbuffer to perform layout transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
		Coral::CommandBufferPtr transitionToPresent;
		
		/// Semaphore to be signaled once the swapchain image memory layout has transitioned to
		/// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR.
		Coral::SemaphorePtr imagePresentableSemaphore;
	};

	VkSemaphore mSemaphore{ VK_NULL_HANDLE };

	std::vector<SwapchainImageData> mSwapchainImageData;

	std::vector<SwapchainSyncObjects> mSwapchainSyncObjects;

	Coral::ImagePtr mSwapchainDepthImage;

	Coral::SwapchainCreateConfig mConfig{};

	Coral::SwapchainImageInfo mCurrentSwapchainImageInfo{};

}; // class Surface

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SWAPCHAINIMPL_HPP
