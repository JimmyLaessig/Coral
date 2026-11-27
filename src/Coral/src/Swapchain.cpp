#include <Coral/Swapchain.h>

#include "Swapchain.hpp"
#include "Context.hpp"

using namespace Coral;

CoSwapchain_T::CoSwapchain_T(std::shared_ptr<Coral::Swapchain> impl)
    : impl(impl)
{
    mImages.resize(impl->swapchainImageCount());
}


CoResult
coContextCreateSwapchain(CoContext context, const CoSwapchainCreateConfig* pConfig, CoSwapchain* pSwapchain)
{
    Swapchain::CreateConfig config{};
    config.nativeWindowHandle = pConfig->nativeWindowHandle;
    config.format             = pConfig->format;
    config.lockToVSync        = pConfig->lockToVSync;
    config.minImageCount      = pConfig->minImageCount;

    auto impl = context->impl->createSwapchain(config);
    if (impl)
    {
        *pSwapchain = new CoSwapchain_T(impl.value());

        return CO_SUCCESS;
    }

    return static_cast<CoResult>(impl.error());
}


void
coDestroySwapchain(CoSwapchain swapchain)
{
    delete swapchain;
}


CoResult
coSwapchainAcquireNextImage(const CoSwapchain swapchain, CoSemaphore signalSemaphore, CoFence signalFence, CoAcquiredImageInfo* pInfo)
{
    auto result = swapchain->impl->acquireNextSwapchainImage(signalSemaphore ? signalSemaphore->impl : nullptr, 
                                                             signalFence ? signalFence->impl : nullptr);

    auto& image = swapchain->mImages[result.index];

    CoResult res = CO_SUCCESS;

    // Update the cached swapchain image if the image has changed since the last acquire.
    // This happens when the swapchain is recreated due to a window resize or other event that
    // invalidates the swapchain.
    if (!image || image->impl != result.image)
    {
        image.reset(new CoImage_T{ result.image });
        res = CO_WARING_SWAPCHAIN_RESIZED;
    }

    pInfo->image = image.get();
    pInfo->index = result.index;

    return res;
}


uint32_t
coSwapchainGetImageCount(const CoSwapchain swapchain)
{
    return swapchain->impl->swapchainImageCount();
}


CoExtent
coSwapchainGetImageExtent(const CoSwapchain swapchain)
{
    return swapchain->impl->swapchainExtent();
}
