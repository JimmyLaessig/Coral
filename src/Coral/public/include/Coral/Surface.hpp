#ifndef CORAL_SURFACE_HPP
#define CORAL_SURFACE_HPP

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

struct SwapchainConfig
{
	PixelFormat format{ Coral::PixelFormat::RGBA8_SRGB };

	std::optional<PixelFormat> depthFormat{ Coral::PixelFormat::DEPTH24_STENCIL8 };

	uint32_t swapchainImageCount{ 3 };
};

struct SurfaceCreateConfig
{
	SwapchainConfig swapchainConfig{};

	void* nativeWindowHandle{ nullptr };
};

enum class SurfaceCreationError
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

class CORAL_API Surface
{

public:
	
	virtual ~Surface() = default;

	virtual void* nativeWindowHandle() = 0;

	virtual SwapchainImageInfo acquireNextSwapchainImage(Fence* fence) = 0;

	virtual SwapchainImageInfo getCurrentSwapchainImage() = 0;

	virtual uint32_t getCurrentSwapchainImageIndex() = 0;

	virtual uint32_t getSwapchainImageCount() const = 0;

	virtual FramebufferSignature getFramebufferSignature() = 0;
};



} // namespace Coral

#endif // !CORAL_SHADERMODULE_HPP