#ifndef CORAL_COMMANDBUFFER_H
#define CORAL_COMMANDBUFFER_H

#include <Coral/CommandQueue.h>
#include <Coral/Image.h>
#include <Coral/Buffer.h>
#include <Coral/PipelineState.h>
#include <Coral/Sampler.h>

#include <Coral/Framebuffer.h>

/// Configuration to create a CommandBuffer
typedef struct 
{
	/// The name of the CommandBuffer
	const char* name;
} CoCommandBufferCreateConfig;


struct CoCommandBuffer_T;

typedef CoCommandBuffer_T* CoCommandBuffer;

CORAL_API CoResult coCommandQueueCreateCommandBuffer(CoCommandQueue queue, 
	                                                 const CoCommandBufferCreateConfig* pConfig, 
	                                                 CoCommandBuffer* pCommandBuffer);

CORAL_API void coDestroyCommandBuffer(CoCommandBuffer buffer);


/// Begin command recording
CORAL_API CoResult coCommandBufferBegin(CoCommandBuffer commandBuffer);

/// End command recording
/**
 * Command buffers can only be scheduled for execution the command buffer
 * is no longer in recording state
 */
CORAL_API CoResult coCommandBufferEnd(CoCommandBuffer commandBuffer);

typedef enum 
{
	Triangles, 
	Lines, 
	Points,
} CoPrimitiveType;

typedef enum
{
	COLOR,
	DEPTH,
	COLOR_AND_DEPTH
} CoBlitAttachment;

///
typedef struct 
{
	/// The index of the attachment to clear
	uint32_t attachment;
	/// The clear operation
	CoClearOp clearOp;
	/// The color
	float color[4];
} CoClearColor;


///
typedef struct
{
	CoClearOp clearOp;
	float depth;
	uint8_t stencil;
} CoClearDepthStencil;


typedef struct 
{
	CoFramebuffer framebuffer;

	CoClearColor* pClearColors;
	
	uint32_t clearColorsCount;

	CoClearDepthStencil* clearDepthStencil;

} CoBeginRenderPassInfo;


CORAL_API CoResult coCommandBufferBeginRenderPass(CoCommandBuffer commandBuffer, const CoBeginRenderPassInfo* beginInfo);

CORAL_API CoResult coCommandBufferEndRenderPass(CoCommandBuffer commandBuffer);

typedef struct 
{
	/// Source buffer to copy data from
	CoBuffer source;

	/// Destination buffer to copy data to
	CoBuffer dest;

	/// Starting offset from the start of the source buffer (in bytes)
	size_t sourceOffset;

	/// Starting offset from the start of the destination buffer (in bytes)
	size_t destOffset;

	/// Number of bytes to copy
	size_t size;
} CoCopyBufferInfo;

typedef struct 
{
	CoBuffer source;

	CoImage dest;
} CoCopyImageInfo;

typedef struct
{
	/// The number of vertices to draw
	uint32_t indexCount;

	/// The base index within the index buffer
	uint32_t firstIndex;
} CoDrawIndexInfo;

typedef enum
{
	CO_VIEWPORT_MODE_Y_UP,
	CO_VIEWPORT_MODE_Y_DOWN
} CoViewportMode;

typedef struct
{
	CoRectangle viewport;
	float minDepth;
	float maxDepth;
	CoViewportMode mode;
} CoViewportInfo;


typedef struct
{
	/// The buffer to update
	CoBuffer buffer;
	/// Byte offset to the begin of the buffer
	uint32_t offset;
	/// The data to be put into the buffer
	const CoByte* pData;
	// Number of bytes in pData
	uint32_t dataCount;
} CoUpdateBufferDataInfo;


typedef struct 
{
	/// The image to update
	CoImage image;

	/// The data to be put into the image
	const CoByte* pData;

	uint32_t dataCount;
} CoUpdateImageDataInfo;


CORAL_API CoResult coCommandBufferUpdateBufferData(CoCommandBuffer commandBuffer, const CoUpdateBufferDataInfo* info);

CORAL_API CoResult coCommandBufferUpdateImageData(CoCommandBuffer commandBuffer, const CoUpdateImageDataInfo* info);

CORAL_API CoResult coCommandBufferBlitImage(CoCommandBuffer commandBuffer, CoImage source, CoImage dest);

CORAL_API CoResult coCommandBufferGenerateMipMaps(CoCommandBuffer commandBuffer, CoImage image);

/// Bind the vertex buffer
/**
 * \param buffer The buffer containing the vertex attribute data
 * \param binding Index of the vertex input whose state is updated by the command
 * \param offset The offset from the base address of the buffer to the first element in bytes
 * \param stride The byte stride between attributes in the buffer. If zero, the elements are assumed to be tightly
 * packed and the stride will be set to the byte size of one element.
 */
CORAL_API CoResult coCommandBufferBindVertexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, uint32_t binding, size_t offset, size_t stride);

/// Bind the index buffer
/**
 * \param buffer The buffer containing the vertex indices
 * \param format The format of the indices contained in the buffer
 * \param offset The offset from the base address of the buffer to the first element in bytes
 */
CORAL_API CoResult coCommandBufferBindIndexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, CoIndexFormat format, size_t offset);

/// Bind the graphics pipeline to the command buffer
CORAL_API CoResult coCommandBufferBindPipeline(CoCommandBuffer commandBuffer, CoPipelineState pipeline);

///
CORAL_API CoResult coCommandBufferSetViewport(CoCommandBuffer commandBuffer, const CoViewportInfo* info);

///
CORAL_API CoResult coCommandBufferBindUniformBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, uint32_t binding);

///
CORAL_API CoResult coCommandBufferBindImage(CoCommandBuffer commandBuffer, CoImage image, uint32_t binding);

///
CORAL_API CoResult coCommandBufferBindSampler(CoCommandBuffer commandBuffer, CoSampler sampler, uint32_t binding);

/// Draw the primitives with indexed vertices
/**
 * \param commandBuffer The command buffer into which the command will be recorded. Must not be null.
 * \param firstIndex
 * \param indexCount
 */
CORAL_API CoResult coCommandBufferDrawIndexed(CoCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t indexCount);

typedef struct
{
	/// The command buffers to execute in batch. 
	/**
	 * The order of command buffers in the list dictates the order of
	 * submission and beginning of execution, but are allowed to proceed
	 * independently after that and complete out of order.
	 *
	 * \note: The caller must ensure that the waitSemaphores are submitted as signalSemaphores of a command buffer
	 * submission. Otherwise, execution will not start.
	 */
	CoCommandBuffer* pCommandBuffers;

	uint32_t commandBufferCount;

	/// List of semaphores to wait for before execution of the command buffer can start.
	CoSemaphore* pWaitSemaphores;

	uint32_t waitSemaphoreCount;

	/// List of semaphores to signal once execution of the command buffer has finished.
	CoSemaphore* pSignalSemaphores;

	uint32_t signalSemaphoreCount;
} CoCommandBufferSubmitInfo;


CORAL_API CoResult coCommandQueueSubmit(CoCommandQueue queue, const CoCommandBufferSubmitInfo* info, CoFence fence);

#endif // !CORAL_COMMANDBUFFER_H
