#ifndef CORAL_FENCE_H
#define CORAL_FENCE_H

#include <Coral/Export.h>
#include <Coral/Context.h>

/*!
 * Structure specifying the parameters of a newly created fence object
 */
typedef struct
{
    /*
     * Flag indicating whether the fence should be created in a signaled state. If true, the fence 
     * will be created as signaled, meaning that it will not block any wait operations until it is
     * reset. If false, the fence will be created in an unsignaled state, and any wait operations
     * will block until the fence is signaled.
     */
    bool createSignaled;
} CoFenceCreateConfig;


struct CoFence_T;

typedef CoFence_T* CoFence;

/*!
 * \brief Create a new fence object
 * \param context Handle to a CoContext object that creates the fence object.
 * \param pConfig Pointer to a CoFenceCreateConfig instance containing parameters affecting the
 *                fence creation.
 * \param[out] pFence Pointer to a CoFence handle in which the resulting fence object is returned
 * \return Returns CO_SUCESS if fence creation succeeded. Otherwise one of the CO_ERROR_* values is
 *         returned.
 */
CORAL_API CoResult coContextCreateFence(CoContext context, const CoFenceCreateConfig* pConfig, CoFence* pFence);

/*!
 * \brief Destroy the fence object
 * \param fence Handle to a CoFence object to destroy
 */
CORAL_API void coDestroyFence(CoFence fence);

/*!
 * \brief Wait for the fence to be signaled
 * \param fence Handle to a CoFence object to destroy
 * \param timeout The maximum time to wait for the fence to be signaled in nanoseconds.
 * \return Returns CO_SUCCESS if the fence was signaled within the specified timeout or
 *         CO_ERROR_TIMEOUT if the timeout was reached before the fence was signaled.
 */
CORAL_API CoResult coFenceWait(CoFence fence, uint64_t timeout);

/*!
 * \brief Reset the fence to an unsignaled state
 * If the fence is already in an unsignaled state, this function has no effect.
 * \param fence Handle to a CoFence object
 */
CORAL_API void coFenceReset(CoFence fence);

#endif // !CORAL_FENCE_H
