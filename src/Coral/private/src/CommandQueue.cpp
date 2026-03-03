#include <Coral/CommandQueue.h>
#include <Coral/CommandQueue.hpp>
#include <Coral/Swapchain.hpp>
#include <Coral/Semaphore.hpp>
using namespace Coral;


CoResult
coCommandQueuePresent(CoCommandQueue queue, const CoPresentInfo* info)
{
    Coral::PresentInfo presentInfo{};
    presentInfo.swapchain = info->swapchain->impl.get();

    std::vector<Coral::SemaphorePtr> waitSemaphores;
    for (auto s : std::span(info->pWaitSemaphores, info->waitSemaphoreCount))
    {
        waitSemaphores.push_back(s->impl);
    }

    presentInfo.waitSemaphores = waitSemaphores;
    return queue->impl->submit(presentInfo) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandQueueWaitIdle(CoCommandQueue queue)
{
    return queue->impl->waitIdle() ? CO_SUCCESS : CO_FAILED;
}

