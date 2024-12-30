#ifndef CORAL_VULKAN_SURFACEIMPL_HPP
#define CORAL_VULKAN_SURFACEIMPL_HPP

#include <Coral/Surface.hpp>

#include "ContextImpl.hpp"

#include <memory>


namespace Coral::Vulkan
{

class SurfaceImpl : public Coral::Surface
{
public:

	virtual ~SurfaceImpl();

	std::optional<Coral::SurfaceCreationError> init(Coral::Vulkan::ContextImpl& context, const Coral::SurfaceCreateConfig& config);

	VkSurfaceKHR getVkSurface();

	VkSwapchainKHR getVkSwapchain();

	void* nativeWindowHandle() override;

	bool acquireNextSwapchainImage(Coral::Fence* fence) override;

	uint32_t getCurrentSwapchainImageIndex() override;

	Coral::Image* getSwapchainImage(uint32_t index) override;

	Coral::Image* getSwapchainDepthImage() override;

	Coral::Framebuffer* getSwapchainFramebuffer(uint32_t index) override;

	SwapchainExtent getSwapchainExtent() const override;

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

	VkSemaphore mSemaphore{ VK_NULL_HANDLE };

	std::vector<SwapchainImageData> mSwapchainImageData;
	Coral::ImagePtr mSwapchainDepthImage;

	Coral::SwapchainConfig mConfig{};

}; // class Surface

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SURFACEIMPL_HPP