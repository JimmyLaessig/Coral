#include <Coral/Semaphore.h>

#include "Semaphore.hpp"
#include "Context.hpp"

using namespace Coral;

CoResult
coContextCreateSemaphore(CoContext context, const CoSemaphoreCreateConfig* pConfig, CoSemaphore* pSemaphore)
{
    auto impl = context->impl->createSemaphore(*pConfig);

    if (impl)
    {
        *pSemaphore = new CoSemaphore_T{ impl.value() };
        return CO_SUCCESS;
    }

    return static_cast<CoResult>(impl.error());
}

void
coDestroySemaphore(CoSemaphore semaphore)
{
    delete semaphore;
}
