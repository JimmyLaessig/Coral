#ifndef CORAL_SWAPCHAIN_H
#define CORAL_SWAPCHAIN_H

#include <Coral/System.hpp>
#include <Coral/Framebuffer.h>
#include <Coral/Semaphore.h>
#include <Coral/Fence.h>

/// Structure specifying the parameters of a newly created swapchain object
typedef struct 
{
	/// Pointer to the native window handle
	void* nativeWindowHandle;
	/// The pixel format of the swapchain images
	CoPixelFormat format;
	/// Pointer to the pixel format of the depth-stencil image. If nullptr, no depth-stencil image is created
	CoPixelFormat* depthFormat;
	/// Minimum number of images in the swapchain
	uint32_t minImageCount;
	/// Flag indicating if the swapchain's refresh rate should be locked to vertical synchronization of the monitor
	bool lockToVSync;
} CoSwapchainCreateConfig;


struct CoSwapchain_T;

typedef CoSwapchain_T* CoSwapchain;

///
/**
 * \param context Handle to a CoContext object
 * \param config Pointer to a CoSwapchainCreateConfig structure
 * \param pSwapchain Pointer to a CoSwapchain handle in which the created swapchain is returned
 */
CORAL_API CoResult coContextCreateSwapchain(CoContext context, const CoSwapchainCreateConfig* pConfig, CoSwapchain* pSwapchain);

/// Destroy the swapchain object
/**
 * \param swapchain The swapchain to destroy
 */
CORAL_API void coDestroySwapchain(CoSwapchain swapchain);


typedef struct
{
	/// The framebuffer used to render to the swapchain image
	CoFramebuffer framebuffer;

	/// Semaphore that is signaled once the swapchain image is ready for use
	/**
	 * Command buffers that render to the swapchain image's framebuffer must wait for this semaphore before beginning
	 * rendering.
	 */
	CoSemaphore imageAcquiredSemaphore;

	/// Index of the swapchain image
	uint32_t index;
} CoSwapchainImageInfo;

/// Get the image count of the swapchain
/**
 * \param Valid handle to a CoSwapchain object
 * \return Returns the number of images in the swapchain
 */
CORAL_API uint32_t coSwapchainGetImageCount(const CoSwapchain swapchain);

/// Get the framebuffer layout
CORAL_API void coSwapchainGetFramebufferLayout(const CoSwapchain, CoFramebufferLayout* pLayout);

/// Acquire the next swapchain image
/**
 * \param swapchain Valid handle to a CoSwapchain object
 * \param fence Optional handle to a CoFence object to be signaled once the image is acquired.
 * \param info Pointer to a CoSwapchainImageInfo structure in which the next swapchain image info is returned
 */
CORAL_API CoResult coSwapchainAcquireNextImage(CoSwapchain swapchain, CoFence fence, CoSwapchainImageInfo* pInfo);

/// Get the current swapchain image info
/**
 * \param swapchain Valid handle to a CoSwapchain object
 * \param info Pointer to a CoSwapchainImageInfo structure in which the swapchain image info is returned
 */
CORAL_API void coSwapchainGetCurrentSwapchainImageInfo(const CoSwapchain swapchain, CoSwapchainImageInfo* pInfo);

#endif // !CORAL_SWAPCHAIN_H
