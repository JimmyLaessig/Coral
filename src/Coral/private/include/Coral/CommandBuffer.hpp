#ifndef CORAL_COMMANDBUFFER_HPP
#define CORAL_COMMANDBUFFER_HPP

#include <Coral/CommandBuffer.h>
#include <Coral/CoralFwd.hpp>

#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <map>

namespace Coral
{

struct ClearColor
{
    CoClearOp clearOp;
    float color[4];
};


struct BeginRenderPassInfo
{
    Framebuffer* framebuffer{ nullptr };

    std::map<uint32_t, ClearColor> clearColor;
    std::optional<CoClearDepthStencil> clearDepth;
};


struct CopyBufferInfo
{
    /// Source buffer to copy data from
    BufferPtr source{ nullptr };

    /// Destination buffer to copy data to
    BufferPtr dest{ nullptr };

    /// Starting offset from the start of the source buffer (in bytes)
    size_t sourceOffset{ 0 };

    /// Starting offset from the start of the destination buffer (in bytes)
    size_t destOffset{ 0 };

    /// Number of bytes to copy
    size_t size{ 0 };
};

struct CopyImageInfo
{
    BufferPtr source{ nullptr };

    ImagePtr dest{ nullptr };
};


struct DrawIndexInfo
{
    /// The number of vertices to draw
    uint32_t indexCount{ 0 };

    /// The base index within the index buffer
    uint32_t firstIndex{ 0 };
};



struct UpdateBufferDataInfo
{
    /// The buffer to update
    Coral::BufferPtr buffer{ nullptr };

    /// Offset to the begin of the buffer
    uint32_t offset{ 0 };

    /// The data to be put into the buffer
    std::span<const std::byte> data;
};


struct UpdateImageDataInfo
{
    /// The image to update
    Coral::ImagePtr image{ nullptr };
    
    /// The data to be put into the image
    std::span<const std::byte> data;
};


class CORAL_API CommandBuffer
{
public:

    using CreateConfig = CoCommandBufferCreateConfig;

    /// Error codes for CommandBuffer creation
    enum class CreateError
    {
        /// CommandBuffer creation failed due to an internal error.
        INTERNAL_ERROR,
    };

    virtual ~CommandBuffer() = default;

    /// Begin command recording
    virtual bool begin() = 0;

    /// End command recording
    /**
     * Command buffers can only be scheduled for execution the command buffer
     * is no longer in recording state
     */
    virtual bool end() = 0;

    /// Clear the color attachments of the bound framebuffer
    virtual bool cmdClearImage(Coral::ImagePtr image, const CoClearColor& clearColor) = 0;

    virtual bool cmdBeginRenderPass(const BeginRenderPassInfo& info) = 0;

    virtual bool cmdEndRenderPass() = 0;

    virtual bool cmdUpdateBufferData(const UpdateBufferDataInfo& info) = 0;

    virtual bool cmdUpdateImageData(const UpdateImageDataInfo& info) = 0;

    virtual bool cmdCopyImage(const CopyImageInfo& info) = 0;

    virtual bool cmdCopyBuffer(const CopyBufferInfo& info) = 0;

    virtual bool cmdGenerateMipMaps(ImagePtr image) = 0;

    /// Bind the vertex buffer
    /**
     * \param buffer The buffer containing the vertex attribute data
     * \param binding Index of the vertex input whose state is updated by the command
     * \param offset The offset from the base address of the buffer to the first element in bytes
     * \param stride The byte stride between attributes in the buffer. If zero, the elements are assumed to be tightly
     * packed and the stride will be set to the byte size of one element.
     */
    virtual bool cmdBindVertexBuffer(Coral::BufferPtr buffer, uint32_t binding, size_t offset, size_t stride) = 0;

    /// Bind the index buffer
    /**
     * \param buffer The buffer containing the vertex indices
     * \param format The format of the indices contained in the buffer
     * \param offset The offset from the base address of the buffer to the first element in bytes
     */
    virtual bool cmdBindIndexBuffer(Coral::BufferPtr buffer, CoIndexFormat format, size_t offset) = 0;

    /// Bind the graphics pipeline to the command buffer
    virtual bool cmdBindPipeline(Coral::PipelineStatePtr pipeline) = 0;

    /// Draw the primitives with indexed vertices
    virtual bool cmdDrawIndexed(const DrawIndexInfo& info) = 0;

    virtual bool cmdSetViewport(const CoViewportInfo& info) = 0;

    virtual void cmdBindDescriptor(Coral::BufferPtr buffer, uint32_t binding) = 0;

    virtual void cmdBindDescriptor(Coral::SamplerPtr sampler, uint32_t binding) = 0;

    virtual void cmdBindDescriptor(Coral::ImagePtr image, uint32_t binding) = 0;

    virtual bool cmdBlitImage(Coral::ImagePtr source, Coral::ImagePtr dest) = 0;
};

} // namespace Coral

struct CoCommandBuffer_T
{
    std::shared_ptr<Coral::CommandBuffer> impl;
};

#endif // !CORAL_COMMANDBUFFER_HPP