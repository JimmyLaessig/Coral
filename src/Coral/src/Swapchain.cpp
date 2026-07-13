#include <Coral/Swapchain.h>

#include "Swapchain.hpp"
#include "Context.hpp"

using namespace Coral;

CoSwapchain_T::CoSwapchain_T(std::shared_ptr<Coral::Swapchain> impl)
    : impl(impl)
{
    mData.resize(impl->swapchainImageCount());
}


CoResult
coContextCreateSwapchain(CoContext context, const CoSwapchainCreateConfig* pConfig, CoSwapchain* pSwapchain)
{
    auto impl = context->impl->createSwapchain(*pConfig);
    if (impl)
    {
        *pSwapchain = new CoSwapchain_T(impl.value());

        (*pSwapchain)->mData.resize(impl.value()->swapchainImageCount());

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
coSwapchainAcquireNextImage(const CoSwapchain swapchain, CoFence fence, CoAcquiredImageInfo* pInfo)
{
    auto result = swapchain->impl->acquireNextSwapchainImage(fence ? fence->impl : nullptr);

    auto& data = swapchain->mData[result.index];

    // Update the cached swapchain image data if the image has changed since the last acquire.
    // This happens when the swapchain is recreated due to a window resize or other event that
    // invalidates the swapchain.
    if (!data.framebuffer || data.framebuffer->impl != result.framebuffer)
    {
        data.framebuffer.reset(new CoFramebuffer_T{ result.framebuffer });
        data.semaphore.reset(new CoSemaphore_T{ result.imageReadySemaphore });
    }

    pInfo->framebuffer            = data.framebuffer.get();
    pInfo->imageAcquiredSemaphore = data.semaphore.get();
    pInfo->index                  = result.index;

    return CO_SUCCESS;
}


uint32_t
coSwapchainGetImageCount(const CoSwapchain swapchain)
{
    return swapchain->impl->swapchainImageCount();
}


//void
//coSwapchainGetCurrentSwapchainImageInfo(const CoSwapchain swapchain, CoSwapchainImageInfo* pInfo)
//{
//    auto index    = swapchain->impl->currentSwapchainImageIndex();
//    auto infoImpl = swapchain->impl->currentSwapchainImage();
//    
//    auto& data = swapchain->mData[index];
//    if (!data.framebuffer || data.framebuffer->impl != infoImpl.framebuffer)
//    {
//        swapchain->mData[index].framebuffer.reset(new CoFramebuffer_T{ infoImpl.framebuffer });
//        swapchain->mData[index].semaphore.reset(new CoSemaphore_T{ infoImpl.imageReadySemaphore });
//        swapchain->mData[index].image.reset(new CoImage_T{ infoImpl.image });
//    }
//
//    pInfo->framebuffer            = swapchain->mData[index].framebuffer.get();
//    pInfo->imageAcquiredSemaphore = swapchain->mData[index].semaphore.get();
//    pInfo->index                  = swapchain->impl->currentSwapchainImageIndex();
//}
