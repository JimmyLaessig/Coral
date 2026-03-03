#ifndef CORAL_SWAPCHAIN_HPP
#define CORAL_SWAPCHAIN_HPP

#include <Coral/Swapchain.h>
#include <Coral/CoralFwd.hpp>
#include <Coral/Framebuffer.hpp>

#include <cstdint>
#include <optional>


namespace Coral
{

struct SwapchainImageInfo
{
	/// The current swapchain image
	Coral::ImagePtr image{ nullptr };

	/// The framebuffer used to render to this swapchain image
	Coral::FramebufferPtr framebuffer{ nullptr };

	/// Semaphore that is signaled once the image is ready for use
	/**
	 * Command buffers that render to the current framebuffer must wait for this semaphore before beginning rendering.
	 */
	Coral::SemaphorePtr imageReadySemaphore{ nullptr };

	/// The extent of the current swapchain image
	/**
	 * The swapchain extent remains the same untik the window is changed
	 */
	CoExtent extent{ 0, 0 };
};

class Swapchain
{

public:

	using CreateConfig = CoSwapchainCreateConfig;

	enum class CreateError
	{
		INTERNAL_ERROR
	};

	virtual ~Swapchain() = default;

	virtual void* nativeWindowHandle() = 0;

	virtual SwapchainImageInfo acquireNextSwapchainImage(FencePtr fence) = 0;

	virtual SwapchainImageInfo currentSwapchainImage() const = 0;

	virtual uint32_t currentSwapchainImageIndex() const = 0;

	virtual uint32_t swapchainImageCount() const = 0;

	virtual CoExtent swapchainExtent() const = 0;

	virtual Framebuffer::Layout framebufferLayout() const = 0;
};

} // namespace Coral

struct SwapchainImageData
{
	std::unique_ptr<CoFramebuffer_T> framebuffer{ nullptr };
	std::unique_ptr<CoImage_T> image{ nullptr };
	std::unique_ptr<CoSemaphore_T> semaphore{ nullptr };
};

struct CoSwapchain_T
{
	CoSwapchain_T(std::shared_ptr<Coral::Swapchain> impl);

	std::shared_ptr<Coral::Swapchain> impl;

	std::vector<SwapchainImageData> mData;
};

#endif // !CORAL_SWAPCHAIN_HPP