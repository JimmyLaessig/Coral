#ifndef CORAL_SWAPCHAIN_HPP
#define CORAL_SWAPCHAIN_HPP

#include <Coral/Fence.hpp>
#include <Coral/Framebuffer.hpp>
#include <Coral/Image.hpp>
#include <Coral/Semaphore.hpp>
#include <Coral/System.hpp>
#include <Coral/Types.hpp>

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
	Coral::Framebuffer* framebuffer{ nullptr };
	Coral::Image* image{ nullptr };
	Coral::Image* depthImage{ nullptr };
	Coral::Semaphore* imageAvailableSemaphore{ nullptr };
	SwapchainExtent extent{ 0, 0 };
};

class CORAL_API Swapchain
{

public:
	
	virtual ~Swapchain() = default;

	virtual void* nativeWindowHandle() = 0;

	virtual SwapchainImageInfo acquireNextSwapchainImage(Fence* fence) = 0;

	virtual SwapchainImageInfo getCurrentSwapchainImage() = 0;

	virtual uint32_t getCurrentSwapchainImageIndex() = 0;

	virtual uint32_t getSwapchainImageCount() const = 0;

	virtual FramebufferSignature getFramebufferSignature() = 0;
};

} // namespace Coral

#endif // !CORAL_SWAPCHAIN_HPP