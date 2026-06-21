#ifndef CORAL_COMMANDBUFFER_H
#define CORAL_COMMANDBUFFER_H

#include <Coral/CommandQueue.h>
#include <Coral/Image.h>
#include <Coral/Buffer.h>
#include <Coral/PipelineState.h>
#include <Coral/Sampler.h>

#include <Coral/Framebuffer.h>

/*!
 * Structure specifying the parameters of a newly created CommandBuffer object
 */
typedef struct 
{
    /*!
     * The name of the CommandBuffer
     */
    const char* name;

    /*!
     * Flag indicating if the CommandBuffer should retain references to resources used in commands submitted to it. If
     * enabled, submitted resources are allowed to be destroyed after CommandBuffer recording. Their lifetime is
     * automatically extended until the CommandBuffer execution has finished. Note, that this does not replace any
     * coDestroy* call and resources must still be destroyed properly to prevent memory leaks.
     */
    bool retainReferences;

} CoCommandBufferCreateConfig;


struct CoCommandBuffer_T;

typedef CoCommandBuffer_T* CoCommandBuffer;

/*!
 * \brief Create a CommandBuffer object
 * \param context Handle to a CoCommandQueue object that creates the CommandBuffer object.
 * \param pConfig Pointer to a CoCommandBufferCreateConfig instance containing parameters affecting the CommandBuffer
 *                creation.
 * \param[out] pCommandBuffer Pointer to a CoCommandBuffer handle in which the resulting CommandBuffer object is 
 *                            returned.
 */
CORAL_API CoResult coCommandQueueCreateCommandBuffer(CoCommandQueue queue, 
                                                     const CoCommandBufferCreateConfig* pConfig, 
                                                     CoCommandBuffer* pCommandBuffer);

/*!
 * \brief Destroy the CommandBuffer object
 * \param buffer Handle to the CoCommandBuffer object to destroy
 */
CORAL_API void coDestroyCommandBuffer(CoCommandBuffer buffer);

/*!
 * \brief Begin command recording
 * \param commandBuffer Handle to the CoCommandBuffer object
 */
CORAL_API CoResult coCommandBufferBegin(CoCommandBuffer commandBuffer);

/*!
 * \brief End command recording 
 * \param commandBuffer Handle to the CoCommandBuffer object
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

/*!
 * Structure containing the clear color of a Framebuffer attachment
 */
typedef struct 
{
    /*!
     * The index of the attachment to clear
     */
    uint32_t attachment;

    /*!
     * The clear operation
     */
    CoClearOp clearOp;

    /*!
     * The color
     */
    float color[4];

} CoClearColor;


///
typedef struct
{
    CoClearOp clearOp;
    float depth;
    uint8_t stencil;
} CoClearDepthStencil;

/*!
 * Structure containing the render pass begin information
 */
typedef struct 
{
    /*!
     * The FrameBuffer containing the attachments that are used within the render pass
     */
    CoFramebuffer framebuffer;

    /*!
     * Pointer to an array of \ref CoClearColor structures containing clear values for each attachment.  
     */
    CoClearColor* pClearColors;
    
    /*!
     * The number of elements in \ref pClearColors
     */
    uint32_t clearColorsCount;

    /*!
     * Pointer to a CoClearDepthStencil structure containing the clear value for the depth-stencil 
     * attachment.
     */
    CoClearDepthStencil* clearDepthStencil;

} CoBeginRenderPassInfo;

/*!
 * \brief Begin a new render pass
 * \param commandBuffer Handle to the CoCommandBuffer object
 * \param beginInfo Pointer to a CoBeginRenderPassInfo instance containing parameters of the render pass.
 */
CORAL_API CoResult coCommandBufferBeginRenderPass(CoCommandBuffer commandBuffer, const CoBeginRenderPassInfo* pBeginInfo);

/*!
 * \brief End the render pass
 * \param commandBuffer Handle to the CoCommandBuffer object
 */
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


typedef struct
{
    CoRectangle viewport;
    float minDepth;
    float maxDepth;
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
 * \param location The vertex attribute whose state is updated by the command
 * \param offset The offset from the base address of the buffer to the first element in bytes
 * \param stride The byte stride between attributes in the buffer. If zero, the elements are assumed to be tightly
 * packed and the stride will be set to the byte size of one element.
 */
CORAL_API CoResult coCommandBufferBindVertexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, uint32_t location, size_t offset, size_t stride);

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

/*!
 * Structure containing the CommandBuffer submit information
 */
typedef struct
{
    /*!
     * The command buffers to execute in batch.  
     * 
     * The order of command buffers in the list dictates the order of
     * submission and beginning of execution, but are allowed to proceed
     * independently after that and complete out of order.
     *
     * \note: The caller must ensure that the waitSemaphores are submitted as signalSemaphores of a command buffer
     * submission. Otherwise, execution will not start.
     */
    CoCommandBuffer* pCommandBuffers;

    /*!
     * The number of elements in \ref pCommandBuffers
     */
    uint32_t commandBufferCount;

    /*! 
     * Pointer to an array of \ref CoSemaphore objects to wait for before execution of the command buffer can start.
     */
    CoSemaphore* pWaitSemaphores;

    /*!
     * The number of elements in \ref pWaitSemaphores
     */
    uint32_t waitSemaphoreCount;

    /*!
     * Pointer to an array of \ref CoSemaphore objects to signal once execution of the command buffer has finished.
     */
    CoSemaphore* pSignalSemaphores;

    /*!
     * The number of elements in \ref pWaitSemaphores
     */
    uint32_t signalSemaphoreCount;

} CoCommandBufferSubmitInfo;

/*!
 * \brief Submit the CommandBuffers for execution
 * \param Handle to a CoCommandQueue object
 * \param Pointer to a 
 */
CORAL_API CoResult coCommandQueueSubmit(CoCommandQueue queue, const CoCommandBufferSubmitInfo* pInfo, CoFence fence);

#endif // !CORAL_COMMANDBUFFER_H
