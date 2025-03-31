#ifndef CORAL_VULKAN_SURFACEIMPL_HPP
#define CORAL_VULKAN_SURFACEIMPL_HPP

#include <Coral/Surface.hpp>

#include "ContextImpl.hpp"

#include <array>
#include <memory>


namespace Coral::Vulkan
{

class SemaphoreImpl;

class SurfaceImpl : public Coral::Surface
{
public:

	virtual ~SurfaceImpl();

	std::optional<Coral::SurfaceCreationError> init(Coral::Vulkan::ContextImpl& context, const Coral::SurfaceCreateConfig& config);

	VkSurfaceKHR getVkSurface();

	VkSwapchainKHR getVkSwapchain();

	void* nativeWindowHandle() override;

	SwapchainImageInfo acquireNextSwapchainImage(Fence* fence) override;

	SwapchainImageInfo getCurrentSwapchainImage() override;

	uint32_t getCurrentSwapchainImageIndex() override;

	uint32_t getSwapchainImageCount() const override;

	FramebufferSignature getFramebufferSignature() override;

	void present(CommandQueueImpl& commandQueue, std::span<Semaphore*> waitSemaphores);

private:

	bool initSwapchain(const Coral::SwapchainConfig& config);

	ContextImpl* mContext{ nullptr };

	void* mNativeWindowHandle{ nullptr };

	VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

	VkSwapchainKHR mSwapchain{ VK_NULL_HANDLE };

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
		Coral::SemaphorePtr imageReadySemaphore;
		Coral::CommandBufferPtr acquireCommandBuffer;
		Coral::CommandBufferPtr presentCommandBuffer;
		Coral::SemaphorePtr acquireSemaphore;
		Coral::SemaphorePtr presentSemaphore;
	};

	VkSemaphore mSemaphore{ VK_NULL_HANDLE };

	std::vector<SwapchainImageData> mSwapchainImageData;

	std::vector<SwapchainSyncObjects> mSwapchainSyncObjects;

	Coral::ImagePtr mSwapchainDepthImage;

	Coral::SwapchainConfig mConfig{};

	Coral::SwapchainImageInfo mCurrentSwapchainImageInfo{};

}; // class Surface

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SURFACEIMPL_HPP