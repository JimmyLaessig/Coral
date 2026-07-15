#ifndef CORAL_SWAPCHAIN_H
#define CORAL_SWAPCHAIN_H

#include <Coral/Export.h>
#include <Coral/Framebuffer.h>
#include <Coral/Semaphore.h>
#include <Coral/Fence.h>

/*!
 * Structure specifying the parameters of a newly created swapchain object
 */
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

/*!
 * \brief Create a new swapchain object
 * \param context Handle to a CoContext object
 * \param config Pointer to a CoSwapchainCreateConfig structure
 * \param pSwapchain Pointer to a CoSwapchain handle in which the created swapchain is returned
 */
CORAL_API CoResult coContextCreateSwapchain(CoContext context, const CoSwapchainCreateConfig* pConfig, CoSwapchain* pSwapchain);

/*!
 * \brief Destroy the swapchain object
 * \param swapchain Handle to a CoSwapchain object to destroy
 */
CORAL_API void coDestroySwapchain(CoSwapchain swapchain);

/*!
 * Structure containing information about the acquired swapchain image
 */
typedef struct
{
    /*!
     * The framebuffer used to render to the swapchain image
     */
    CoFramebuffer framebuffer;

    /*!
     * Semaphore that is signaled once the swapchain image is ready for use
     * Command buffers that render to the swapchain's framebuffer must wait for this
     * semaphore before beginning rendering.
     */
    CoSemaphore imageAcquiredSemaphore;

    /*!
     * Index of the swapchain image
     */
    uint32_t index;

} CoAcquiredImageInfo;

/*!
 * \brief Get the number of images in the swapchain
 * \param Valid handle to a CoSwapchain object
 * \return Returns the number of images in the swapchain
 */
CORAL_API uint32_t coSwapchainGetImageCount(const CoSwapchain swapchain);

/*!
 * \brief Acquire the next swapchain image 
 * \param swapchain Valid handle to a CoSwapchain object
 * \param signalSemaphore Optional handle to a CoSemaphore object to be signaled once the image
 *        is acquired.
 * \param signalFence Optional handle to a CoFence object to be signaled once the image is acquired.
 * \param info Pointer to a CoAcquiredImageInfo structure in which the framebuffer of the next
 *        swapchain image info is returned
 */
CORAL_API CoResult coSwapchainAcquireNextImage(CoSwapchain swapchain, CoSemaphore signalSemaphore,  CoFence signalFence, CoAcquiredImageInfo* pInfo);

#endif // !CORAL_SWAPCHAIN_H
