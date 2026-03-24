#ifndef CORAL_FENCE_H
#define CORAL_FENCE_H

#include <Coral/System.hpp>
#include <Coral/Context.h>

typedef struct
{

} CoFenceCreateConfig;


struct CoFence_T;

typedef CoFence_T* CoFence;

CORAL_API CoResult coContextCreateFence(CoContext context, const CoFenceCreateConfig* pConfig, CoFence* pFence);

CORAL_API void coDestroyFence(CoFence fence);

CORAL_API CoResult coFenceWait(CoFence fence);

CORAL_API CoResult coFenceReset(CoFence fence);

#endif // !CORAL_FENCE_H