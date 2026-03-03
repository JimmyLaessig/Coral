#ifndef CORAL_SEMAPHORE_H
#define CORAL_SEMAPHORE_H

#include <Coral/Context.h>


typedef struct
{

} CoSemaphoreCreateConfig;

struct CoSemaphore_T;

typedef CoSemaphore_T* CoSemaphore;

CORAL_API CoResult coContextCreateSemaphore(CoContext context, const CoSemaphoreCreateConfig* pConfig, CoSemaphore* pSemaphore);

CORAL_API  void coDestroySemaphore(CoSemaphore Semaphore);

#endif // !CORAL_SEMAPHORE_H
