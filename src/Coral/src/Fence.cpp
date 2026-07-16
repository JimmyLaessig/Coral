#include <Coral/Fence.h>

#include "Fence.hpp"
#include "Context.hpp"

#include <memory>

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
coFenceWait(CoFence fence, uint64_t timeout)
{
    return static_cast<CoResult>(fence->impl->wait(timeout));
}


void
coFenceReset(CoFence fence)
{
    fence->impl->reset();
}
