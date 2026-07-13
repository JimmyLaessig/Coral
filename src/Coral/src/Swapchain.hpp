#ifndef CORAL_SWAPCHAIN_HPP
#define CORAL_SWAPCHAIN_HPP

#include <Coral/Swapchain.h>
#include <Coral/Image.h>

#include "CoralFwd.hpp"
#include "Framebuffer.hpp"
#include "Image.hpp"

#include <cstdint>
#include <optional>


namespace Coral
{

/*!
 *
 */
struct AcquiredImageInfo
{
    /*!
     * The index of the current swapchain image
     */
    uint32_t index{ 0 };

    /*!
     * The framebuffer used to render to this swapchain image
     */
    Coral::FramebufferPtr framebuffer{ nullptr };

    /*!
     * Semaphore that is signaled once the image is ready for use 
     * Command buffers that render to the current framebuffer must wait for this semaphore before
     * beginning rendering.
     */
    Coral::SemaphorePtr imageReadySemaphore{ nullptr };

}; // struct AcquiredImageInfo

/*!
 *
 */
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

    virtual AcquiredImageInfo acquireNextSwapchainImage(FencePtr fence) = 0;

    virtual uint32_t currentSwapchainImageIndex() const = 0;

    virtual uint32_t swapchainImageCount() const = 0;

    virtual CoExtent swapchainExtent() const = 0;

}; // class Swapchain

} // namespace Coral

struct AcquiredImageInfoStorage
{
    std::unique_ptr<CoFramebuffer_T> framebuffer;
    std::unique_ptr<CoSemaphore_T> semaphore;
}; // struct AcquiredImageInfoStorage

struct CoSwapchain_T
{
    CoSwapchain_T(std::shared_ptr<Coral::Swapchain> impl);

    std::shared_ptr<Coral::Swapchain> impl;

    std::vector<AcquiredImageInfoStorage> mData;

}; // struct CoSwapchain_T

#endif // !CORAL_SWAPCHAIN_HPP
