#ifndef CORAL_SWAPCHAIN_HPP
#define CORAL_SWAPCHAIN_HPP

#include <Coral/System.hpp>
#include <Coral/CoralFwd.hpp>
#include <Coral/Types.hpp>
#include <Coral/Framebuffer.hpp>

#include <cstdint>
#include <optional>


namespace Coral
{

struct SwapchainCreateConfig
{
	void* nativeWindowHandle{ nullptr };

	PixelFormat format{ Coral::PixelFormat::RGBA8_SRGB };

	std::optional<PixelFormat> depthFormat{ Coral::PixelFormat::DEPTH24_STENCIL8 };

	uint32_t swapchainImageCount{ 3 };

	bool lockToVSync{ false };
};

enum class SwapchainCreationError
{
	INTERNAL_ERROR
};

struct SwapchainExtent
{
	uint32_t width;
	uint32_t height;
};

struct SwapchainImageInfo
{
	/// The framebuffer used to render to this swapchain image
	Coral::Framebuffer* framebuffer{ nullptr };

	/// Semaphore that is signaled once the image is ready for use
	/**
	 * Command buffers that render to the current framebuffer must wait for this semaphore before beginning rendering.
	 */
	Coral::Semaphore* imageAvailableSemaphore{ nullptr };

	/// The extent of the current swapchain image
	/**
	 * The swapchain extent remains the same untik the window is changed
	 */
	SwapchainExtent extent{ 0, 0 };
};

class CORAL_API Swapchain
{

public:

	virtual ~Swapchain() = default;

	virtual void* nativeWindowHandle() = 0;

	virtual SwapchainImageInfo acquireNextSwapchainImage(Fence* fence) = 0;

	virtual SwapchainImageInfo currentSwapchainImage() const = 0;

	virtual uint32_t currentSwapchainImageIndex() const = 0;

	virtual uint32_t swapchainImageCount() const = 0;

	virtual SwapchainExtent swapchainExtent() const = 0;

	virtual FramebufferSignature framebufferSignature() const = 0;
};

} // namespace Coral

#endif // !CORAL_SWAPCHAIN_HPP