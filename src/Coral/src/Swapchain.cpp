#include <Coral/Swapchain.h>

#include "Swapchain.hpp"
#include "Context.hpp"

using namespace Coral;

CoSwapchain_T::CoSwapchain_T(std::shared_ptr<Coral::Swapchain> impl)
    : impl(impl)
{
    mFramebuffers.resize(impl->swapchainImageCount());
}


CoResult
coContextCreateSwapchain(CoContext context, const CoSwapchainCreateConfig* pConfig, CoSwapchain* pSwapchain)
{
    auto impl = context->impl->createSwapchain(*pConfig);
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

    auto& framebuffer = swapchain->mFramebuffers[result.index];

    // Update the cached swapchain image framebuffer if the image has changed since the last acquire.
    // This happens when the swapchain is recreated due to a window resize or other event that
    // invalidates the swapchain.
    if (!framebuffer || framebuffer->impl != result.framebuffer)
    {
        framebuffer.reset(new CoFramebuffer_T{ result.framebuffer });
    }

    pInfo->framebuffer            = framebuffer.get();
    pInfo->index                  = result.index;

    return CO_SUCCESS;
}


uint32_t
coSwapchainGetImageCount(const CoSwapchain swapchain)
{
    return swapchain->impl->swapchainImageCount();
}
