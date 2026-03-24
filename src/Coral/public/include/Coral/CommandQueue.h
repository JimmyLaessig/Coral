#ifndef CORAL_COMMANDQUEUE_H
#define CORAL_COMMANDQUEUE_H

#include <Coral/Context.h>
#include <Coral/Semaphore.h>
#include <Coral/Swapchain.h>

#include <expected>
#include <span>

struct CoCommandQueue_T;

typedef CoCommandQueue_T* CoCommandQueue;

CORAL_API CoResult coContextGetGraphicsQueue(const CoContext context, CoCommandQueue* pQueue);

CORAL_API CoResult coContextGetComputeQueue(const CoContext context, CoCommandQueue* pQueue);

CORAL_API CoResult coContextGetTransferQueue(const CoContext context, CoCommandQueue* pQueue);

typedef struct
{
	/// List of semaphores to wait for before presentation can start.
	/**
	 * \note: The caller must ensure that the waitSemaphores are submitted as signalSemaphores of a command buffer
	 * submission. Otherwise, execution will not start.
	 */
	CoSemaphore* pWaitSemaphores;

	uint32_t waitSemaphoreCount;

	/// The swapchain to present
	CoSwapchain swapchain;

} CoPresentInfo;

CORAL_API CoResult coCommandQueuePresent(CoCommandQueue queue, const CoPresentInfo* pInfo);

CORAL_API CoResult coCommandQueueWaitIdle(CoCommandQueue queue);

#endif // !CORAL_COMMANDQUEUE_H
