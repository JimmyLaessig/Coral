#ifndef CORAL_VULKAN_SWAPCHAINIMPL_HPP
#define CORAL_VULKAN_SWAPCHAINIMPL_HPP

#include <Coral/SwapchainBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/FenceImpl.hpp>
#include <Coral/Vulkan/FramebufferImpl.hpp>
#include <Coral/Vulkan/ImageImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>
#include <Coral/Vulkan/SemaphoreImpl.hpp>
#include <Coral/Vulkan/CommandQueueImpl.hpp>

#include <array>
#include <memory>


namespace Coral::Vulkan
{

class SemaphoreImpl;

class SwapchainImpl : public Coral::SwapchainBase
{
public:

	using SwapchainBase::SwapchainBase;

	virtual ~SwapchainImpl();

	std::optional<Coral::SwapchainCreationError> init(const Coral::SwapchainCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

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

	Coral::SwapchainCreateConfig mConfig{};

	Coral::SwapchainImageInfo mCurrentSwapchainImageInfo{};

}; // class Surface

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SWAPCHAINIMPL_HPP
