#ifndef CORAL_BUFFER_H
#define CORAL_BUFFER_H

#include <Coral/Context.h>

typedef enum
{
	/// The buffer is used as a vertex buffer
	CO_BUFFER_TYPE_VERTEX  = 0,
	/// The buffer is used as an index buffer
	CO_BUFFER_TYPE_INDEX   = 1,
	/// The buffer is used as uniform buffer
	CO_BUFFER_TYPE_UNIFORM = 2,
	/// The buffer is used as storage buffer
	CO_BUFFER_TYPE_STORAGE = 3,
} CoBufferType;

// Structure specifying the parameters of a newly created buffer object
typedef struct
{
	/// The size of the buffer in bytes
	uint64_t size;

	/// The type of the buffer
	CoBufferType type;

	/// Flag indicating if the buffer's memory is mapped to CPU memory
	bool cpuVisible;
} CoBufferCreateConfig;


struct CoBuffer_T;

typedef CoBuffer_T* CoBuffer;

/// Create a new buffer object
/**
 * \param context The context that creates the buffer object
 * \param pConfig Pointer to a CoBufferCreateConfig instance containing parameters affecting the framebuffer creation.
 * \param[out] pBuffer Pointer to a CoBuffer handle in which the resulting buffer object is returned.
 * \return Returns CO_SUCESS if buffer creation succeeded. Otherwise one of the CO_BUFFER_CREATION_ERROR_* values is
           returned.
 */
CORAL_API CoResult coContextCreateBuffer(CoContext context, const CoBufferCreateConfig* pConfig, CoBuffer* pBuffer);

/// Destroy the buffer object
/**
 * \param buffer The buffer to destroy
 */
CORAL_API void coDestroyBuffer(CoBuffer buffer);

/// Get the size of the buffer in bytes
CORAL_API uint64_t coBufferGetSize(const CoBuffer buffer);

/// Get the size of the buffer in bytes
CORAL_API CoBufferType coBufferGetType(const CoBuffer buffer);

/// Map the buffer memory to CPU-accessible memory
/**
 * This function is successful only if the buffer was created with the `cpuVisible` flag enabled.
 * The CPU <-> GPU memory synchronization functions `coBufferMap` and `coBufferUnMap` should not be called
 * whilst the buffer is in use by a command buffer. Updates to the buffer's CPU data are only synchronized with the
 * buffer's GPU data once `coBufferUnMap` is called.
 * 
 * \param buffer Handle to a CoBuffer object
 * \param pBytes Pointer to a CoByte* variable in which a cpu-accessible pointer to the beginning of the mapped buffer
 *        is returned.
 * \return Returns CO_SUCCESS if the buffer was mapped successfully, otherwise CO_FAILED. 
 */
CORAL_API CoResult coBufferMap(CoBuffer buffer, CoByte** pBytes);

/// Unmap the buffer memory, mapped previously by `map()`.
/*
 * This function is successful only if the buffer was created with the `cpuVisible` flag enabled and a previous call to
 * coBufferMap was successful.
 * The CPU <-> GPU memory synchronization functions `coBufferMap` and `coBufferUnMap` should not be called
 * whilst the buffer is in use by a command buffer. Updates to the buffer's CPU data are only synchronized with the
 * buffer's GPU data once `coBufferUnMap` is called.
 */
CORAL_API CoResult coBufferUnMap(CoBuffer buffer);

#endif // !CORAL_BUFFER_H
