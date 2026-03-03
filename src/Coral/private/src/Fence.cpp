#include <Coral/Fence.h>
#include <Coral/Fence.hpp>
#include <Coral/Context.hpp>

#include <memory>
#include <mutex>

using namespace Coral;


CoResult
coContextCreateFence(CoContext context, const CoFenceCreateConfig* pConfig, CoFence* pFence)
{
    auto impl = context->impl->createFence(*pConfig);
    if (impl)
    {
        *pFence = new CoFence_T{ impl.value() };
        return CO_SUCCESS;
    }

    return static_cast<CoResult>(impl.error());
}


void
coDestroyFence(CoFence fence)
{
    delete fence;
}


CoResult
coFenceWait(CoFence fence)
{
    return fence->impl->wait() ? CO_SUCCESS : CO_FAILED;
}


CoResult
coFenceReset(CoFence fence)
{
    fence->impl->reset();
    return CO_SUCCESS;
}

