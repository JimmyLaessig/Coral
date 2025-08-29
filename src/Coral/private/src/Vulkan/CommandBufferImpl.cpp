#include <Coral/Vulkan/CommandBufferImpl.hpp>

#include <Coral/Vulkan/BufferImpl.hpp>
#include <Coral/Vulkan/CommandQueueImpl.hpp>
#include <Coral/Vulkan/FramebufferImpl.hpp>
#include <Coral/Vulkan/ImageImpl.hpp>
#include <Coral/Vulkan/PipelineStateImpl.hpp>
#include <Coral/Vulkan/SamplerImpl.hpp>

#include <Coral/Visitor.hpp>

#include <optional>
#include <ranges>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

std::optional<VkIndexType>
convert(Coral::AttributeFormat format)
{
    switch (format)
    {
        case Coral::AttributeFormat::UINT32:
            return VK_INDEX_TYPE_UINT32;
        case Coral::AttributeFormat::UINT16:
            return VK_INDEX_TYPE_UINT16;
        default:
            return {};
    }
}


VkAttachmentLoadOp
convert(Coral::ClearOp clearOp)
{
    switch (clearOp)
    {
    case Coral::ClearOp::DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case Coral::ClearOp::LOAD:      return VK_ATTACHMENT_LOAD_OP_LOAD;
    case Coral::ClearOp::CLEAR:     return VK_ATTACHMENT_LOAD_OP_CLEAR;
    default:
        assert(false);
        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}

} // namespace

CommandBufferImpl::CommandBufferImpl(CommandQueueImpl& commandQueue)
    : Resource(commandQueue.context())
    , mCommandQueue(commandQueue)
{
}


CommandBufferImpl::~CommandBufferImpl()
{
    if (mCommandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(context().getVkDevice(), mCommandQueue.getVkCommandPool(), 1, &mCommandBuffer);
    }
}


bool
CommandBufferImpl::init(const Coral::CommandBufferCreateConfig& config)
{
    mName        = config.name;
    auto device  = context().getVkDevice();

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool        = mCommandQueue.getVkCommandPool();
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(device, &allocInfo, &mCommandBuffer) != VK_SUCCESS)
    {
        return false;
    }
    
    return true;
}


bool
CommandBufferImpl::begin()
{
    VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    
    return vkBeginCommandBuffer(mCommandBuffer, &info) == VK_SUCCESS;
}


bool
CommandBufferImpl::end()
{
    return vkEndCommandBuffer(mCommandBuffer) == VK_SUCCESS;
}


bool
CommandBufferImpl::cmdBeginRenderPass(const Coral::BeginRenderPassInfo& info)
{
    if (info.framebuffer == nullptr)
    {
        return false;
    }
    
    auto framebuffer = static_cast<Coral::Vulkan::FramebufferImpl*>(info.framebuffer);

    const auto& colorAttachments = framebuffer->colorAttachments();
    const auto& depthAttachment  = framebuffer->depthAttachment();

    if (colorAttachments.size() != info.clearColor.size())
    {
        return false;
    }

    if (depthAttachment.has_value() != info.clearDepth.has_value())
    {
        return false;
    }

    std::vector<VkRenderingAttachmentInfo> attachments;
    for (const auto& [colorAttachment, clearColor] : std::views::zip(colorAttachments, info.clearColor))
    {
        VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
 
        auto image = static_cast<Coral::Vulkan::ImageImpl*>(colorAttachment.image);

        attachmentInfo.loadOp                      = ::convert(clearColor.clearOp);
        attachmentInfo.clearValue.color.float32[0] = clearColor.color[0];
        attachmentInfo.clearValue.color.float32[1] = clearColor.color[1];
        attachmentInfo.clearValue.color.float32[2] = clearColor.color[2];
        attachmentInfo.clearValue.color.float32[3] = clearColor.color[3];

        attachmentInfo.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentInfo.imageLayout      = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        attachmentInfo.imageView        = image->getVkImageView();
        attachmentInfo.resolveMode      = VK_RESOLVE_MODE_NONE;
        attachmentInfo.resolveImageView = VK_NULL_HANDLE;

        attachments.push_back(attachmentInfo);
    }

    VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    renderingInfo.colorAttachmentCount     = static_cast<uint32_t>(attachments.size());
    renderingInfo.pColorAttachments        = attachments.data();
    renderingInfo.layerCount               = 1;
    renderingInfo.renderArea.offset.x      = 0;
    renderingInfo.renderArea.offset.y      = 0;
    renderingInfo.renderArea.extent.width  = framebuffer->width();
    renderingInfo.renderArea.extent.height = framebuffer->height();

    VkRenderingAttachmentInfo depthAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };

    if (depthAttachment)
    {
        auto image = static_cast<Coral::Vulkan::ImageImpl*>(depthAttachment->image);
        
        // Clear the depth attachment
        depthAttachmentInfo.loadOp                          = ::convert(info.clearDepth->clearOp);
        depthAttachmentInfo.clearValue.depthStencil.depth   = info.clearDepth->depth;
        depthAttachmentInfo.clearValue.depthStencil.stencil = info.clearDepth->stencil;

        depthAttachmentInfo.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentInfo.imageLayout      = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.imageView        = image->getVkImageView();
        depthAttachmentInfo.resolveMode      = VK_RESOLVE_MODE_NONE;
        depthAttachmentInfo.resolveImageView = VK_NULL_HANDLE;

        renderingInfo.pDepthAttachment   = &depthAttachmentInfo;
        renderingInfo.pStencilAttachment = &depthAttachmentInfo;
    }

    vkCmdBeginRendering(mCommandBuffer, &renderingInfo);

    return true;
}


bool
CommandBufferImpl::cmdEndRenderPass()
{
    vkCmdEndRendering(mCommandBuffer);

    return true;
}





bool
CommandBufferImpl::cmdCopyBuffer(const CopyBufferInfo& info)
{
    auto source = static_cast<Coral::Vulkan::BufferImpl*>(info.source);
    auto dest   = static_cast<Coral::Vulkan::BufferImpl*>(info.dest);

    VkBufferCopy bufferCopy;
    bufferCopy.srcOffset = info.sourceOffset;
    bufferCopy.dstOffset = info.destOffset;
    bufferCopy.size      = info.size;
    vkCmdCopyBuffer(mCommandBuffer, source->getVkBuffer(), dest->getVkBuffer(), 1, &bufferCopy);

    return true;
}


bool
CommandBufferImpl::cmdCopyImage(const CopyImageInfo& info)
{
    //auto buffer = static_cast<CO
    //vkCmdCopyBufferToImage(mCommandBuffer, info.source)
    return false;
}


bool
CommandBufferImpl::cmdBindVertexBuffer(Coral::Buffer* buffer, uint32_t binding, size_t offset, size_t stride)
{
    if (buffer->type() != BufferType::VERTEX_BUFFER)
    {
        return false;
    }

    auto bufferImpl       = static_cast<Coral::Vulkan::BufferImpl*>(buffer);
    auto vkBuffer         = bufferImpl->getVkBuffer();
    VkDeviceSize vkOffset = offset;
    VkDeviceSize vkSize   = bufferImpl->size();
    VkDeviceSize vkStride = stride;

    vkCmdBindVertexBuffers2(mCommandBuffer, binding, 1, &vkBuffer, &vkOffset, &vkSize, &vkStride);

    return true;
}


bool
CommandBufferImpl::cmdBindIndexBuffer(Coral::Buffer* buffer, IndexFormat format, size_t offset)
{  
    if (buffer->type() != BufferType::INDEX_BUFFER)
    {
        // TODO buffer type must be index buffer
        return false;
    }

    auto bufferImpl = static_cast<Coral::Vulkan::BufferImpl*>(buffer);

    VkIndexType indexType{};
    switch (format)
    {
        case IndexFormat::UINT16 :
            indexType = VK_INDEX_TYPE_UINT16;
            break;
        case IndexFormat::UINT32:
            indexType = VK_INDEX_TYPE_UINT32;
            break;
        default:
            std::unreachable();
    }

    vkCmdBindIndexBuffer(mCommandBuffer, bufferImpl->getVkBuffer(), offset, indexType);

    return true;
}


bool
CommandBufferImpl::cmdBindPipeline(Coral::PipelineState* pipelineState)
{
    mLastBoundPipelineState = static_cast<Coral::Vulkan::PipelineStateImpl*>(pipelineState);
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mLastBoundPipelineState->getVkPipeline());

    return true;
}


bool
CommandBufferImpl::cmdDrawIndexed(const DrawIndexInfo& info)
{
    cmdBindCachedDescriptors();
    vkCmdDrawIndexed(mCommandBuffer, info.indexCount, 1, info.firstIndex, 0, 0);

    return true;
}


bool
CommandBufferImpl::cmdSetViewport(const Coral::ViewportInfo& info)
{
    if (info.maxDepth < info.minDepth)
    {
        return false;
    }

    if (info.viewport.width == 0 || info.viewport.height == 0)
    {
        return false;
    }

    VkViewport viewport;
    viewport.x        = static_cast<float>(info.viewport.x);
    viewport.y        = static_cast<float>(info.viewport.y);
    viewport.width    = static_cast<float>(info.viewport.width);
    viewport.height   = static_cast<float>(info.viewport.height);
    viewport.minDepth = static_cast<float>(info.minDepth);
    viewport.maxDepth = static_cast<float>(info.maxDepth);

    // Vulkan and OpenGL use different coordinate systems. Per default, the Vulkan coordinate system is y-down. To
    // combat the inverted image, we need to flip the viewport by passing negative height and adjust the y-offset.
    if (info.mode == Coral::ViewportMode::Y_UP)
    {
        viewport.x      = info.viewport.x;
        viewport.y      = viewport.height + viewport.y;
        viewport.width  = viewport.width;
        viewport.height = -viewport.height;
    }

    VkRect2D rect;
    rect.extent.width  = info.viewport.width;
    rect.extent.height = info.viewport.height;
    rect.offset.x      = info.viewport.x;
    rect.offset.y      = info.viewport.y;

    vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(mCommandBuffer, 0, 1, &rect);

    return true;
}


bool
CommandBufferImpl::cmdUpdateBufferData(const Coral::UpdateBufferDataInfo& info)
{
    if (info.offset + info.data.size() > info.buffer->size())
    {
        return false;
    }

    auto buffer = static_cast<Coral::Vulkan::BufferImpl*>(info.buffer);

    auto stagingBuffer = context().requestStagingBuffer(info.data.size());
    auto stagingBufferVK = static_cast<Coral::Vulkan::BufferImpl*>(stagingBuffer.get());

    auto mapped = stagingBuffer->map();
    std::memcpy(mapped + info.offset, info.data.data(), info.data.size());
    stagingBuffer->unmap();

    VkBufferCopy bufferCopy;
    bufferCopy.srcOffset = 0;
    bufferCopy.dstOffset = info.offset;
    bufferCopy.size      = info.data.size();

    vkCmdCopyBuffer(mCommandBuffer, stagingBufferVK->getVkBuffer(), buffer->getVkBuffer(), 1, &bufferCopy);

    /*VkBufferMemoryBarrier barrier{};
    barrier.buffer = bufferImpl->getVkBuffer();
    barrier.dstQueueFamilyIndex = context().getQueueFamilyIndex();
    barrier.srcQueueFamilyIndex = context().getQueueFamilyIndex();*/

    /*VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.image = image->getVkImage();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = image->getMipLevels();*/

    //imageBarriers.push_back(barrier);

    //vkCmdPipelineBarrier(mCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, )
    // Store the temporary staging buffer until the command buffer was executed
    mStagingBuffers.push_back(std::move(stagingBuffer));

    return true;
}


bool
CommandBufferImpl::cmdClearImage(Coral::Image* image, const Coral::ClearColor& clearColor)
{
    if (image->presentable())
    {
        return false;
    }

    auto imageImpl = static_cast<Coral::Vulkan::ImageImpl*>(image);

    VkClearColorValue color;
    color.float32[0] = clearColor.color[0];
    color.float32[1] = clearColor.color[1];
    color.float32[2] = clearColor.color[2];
    color.float32[3] = clearColor.color[3];

    std::vector<VkImageSubresourceRange> ranges;

    for (uint32_t i = 0; i < image->getMipLevels(); ++i)
    {
        auto& range = ranges.emplace_back();
        range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        range.baseMipLevel = i;
        range.levelCount = image->getMipLevels();
    }

    vkCmdClearColorImage(mCommandBuffer,
        imageImpl->getVkImage(),
        imageImpl->getPreferredImageLayout(),
        &color,
        static_cast<uint32_t>(ranges.size()),
        ranges.data());

    return true;
}


bool
CommandBufferImpl::cmdUpdateImageData(const Coral::UpdateImageDataInfo& info)
{
    auto image = static_cast<Coral::Vulkan::ImageImpl*>(info.image);

    auto stagingBuffer   = context().requestStagingBuffer(info.data.size());
    auto stagingBufferVK = static_cast<Coral::Vulkan::BufferImpl*>(stagingBuffer.get());

    auto mapped = stagingBuffer->map();
    std::memcpy(mapped, info.data.data(), info.data.size());
    stagingBuffer->unmap();

    // Before copying data, transition the image layout to be optimal for receiving data. 
    ImageImpl::cmdTransitionImageLayout(mCommandBuffer, 
                                        *image, 
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                                        0, 
                                        image->getMipLevels(), 
                                        VK_ACCESS_TRANSFER_READ_BIT,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy copy{};
    copy.bufferOffset       = 0;
    copy.bufferRowLength    = 0; // TODO: Support non-tightly packed images
    copy.bufferImageHeight  = 0; // TODO: Support non-tightly packed images
    copy.imageExtent.width  = image->width();
    copy.imageExtent.height = image->height();
    copy.imageExtent.depth  = 1;
    copy.imageOffset.x      = 0;
    copy.imageOffset.y      = 0;
    copy.imageOffset.z      = 0;

    // Copy the data to the first mip level
    copy.imageSubresource.mipLevel       = 0;
    copy.imageSubresource.layerCount     = 1;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;

    vkCmdCopyBufferToImage(mCommandBuffer, stagingBufferVK->getVkBuffer(), image->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    mStagingBuffers.push_back(std::move(stagingBuffer));

    // Finally, transition the image layout back
    ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                        *image,
                                        image->getPreferredImageLayout(),
                                        0,
                                        image->getMipLevels(),
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    return true;
}


bool
CommandBufferImpl::cmdGenerateMipMaps(Coral::Image* image)
{
    auto impl = static_cast<ImageImpl*>(image);

    auto levels = image->getMipLevels();
    if (levels == 1)
    {
        return false;
    }


    for (uint32_t i = 1; i < levels; ++i)
    {
        // Transition the image layout of the source and destination image to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL and 
        // K_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
        ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                            *impl,
                                            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                            i - 1,
                                            1,
                                            VK_ACCESS_TRANSFER_WRITE_BIT,
                                            VK_ACCESS_SHADER_READ_BIT,
                                            VK_PIPELINE_STAGE_TRANSFER_BIT,
                                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                    *impl,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    i,
                                    1,
                                    VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_ACCESS_SHADER_READ_BIT,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        VkImageBlit imageBlit{};
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel   = i - 1;
        imageBlit.srcOffsets[1].x           = int32_t(image->width() >> (i - 1));
        imageBlit.srcOffsets[1].y           = int32_t(image->height() >> (i - 1));
        imageBlit.srcOffsets[1].z           = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel   = i;
        imageBlit.dstOffsets[1].x           = int32_t(image->width() >> i);
        imageBlit.dstOffsets[1].y           = int32_t(image->height() >> i);
        imageBlit.dstOffsets[1].z           = 1;

        vkCmdBlitImage(mCommandBuffer, impl->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       impl->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
    }

    // Finally, transition all levels back to the preferred image layout
    ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                        *impl,
                                        impl->getPreferredImageLayout(),
                                        0,
                                        levels,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    return true;
}


void
CommandBufferImpl::cmdBindDescriptor(Coral::Buffer* buffer, uint32_t binding)
{
    VkDescriptorBufferInfo info{};

    info.buffer = static_cast<BufferImpl*>(buffer)->getVkBuffer();
    info.offset = 0.f;
    info.range  = buffer->size();

    mCachedDescriptorInfos[binding] = info;
}


void
CommandBufferImpl::cmdBindDescriptor(Coral::Image* image, Coral::Sampler* sampler, uint32_t binding)
{
    auto imageImpl   = static_cast<ImageImpl*>(image);
    auto samplerImpl = static_cast<SamplerImpl*>(sampler);

    VkDescriptorImageInfo info{};
    info.sampler     = samplerImpl ? samplerImpl->getVkSampler() : VK_NULL_HANDLE;
    info.imageView   = imageImpl ? imageImpl->getVkImageView() : VK_NULL_HANDLE;
    info.imageLayout = imageImpl->getPreferredImageLayout();

    mCachedDescriptorInfos[binding] = info;
}


void
CommandBufferImpl::cmdBindDescriptor(Coral::Image* image, uint32_t binding)
{
    cmdBindDescriptor(image, nullptr, binding);
}


void
CommandBufferImpl::cmdBindDescriptor(Coral::Sampler* sampler, uint32_t binding)
{
    cmdBindDescriptor(nullptr, sampler, binding);
}


void
CommandBufferImpl::cmdBindCachedDescriptors()
{
    if (!mLastBoundPipelineState)
    {
        return;
    }

    auto layout       = mLastBoundPipelineState->getVkPipelineLayout();
    auto bindingPoint = mLastBoundPipelineState->getVkPipelineBindingPoint();

    mDescriptorWrites.clear();
   
    for (const auto& [binding, info] : mCachedDescriptorInfos)
    {
        VkWriteDescriptorSet descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };// = descriptorWrites.emplace_back();
        descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstBinding      = binding;
        descriptorWrite.dstSet          = VK_NULL_HANDLE;
        descriptorWrite.descriptorCount = 1;

        std::visit(Visitor{
            [&](const VkDescriptorBufferInfo& info)
            {
                descriptorWrite.pBufferInfo    = &info;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            },
            [&](const VkDescriptorImageInfo& info)
            {
                descriptorWrite.pImageInfo = &info;
                if (info.sampler && info.imageView)
                {
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                }
                else if (info.sampler)
                {
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                }
                else if (info.imageView)
                {
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                }
            }
        }, info);

        mDescriptorWrites.push_back(descriptorWrite);
    }

    if (!mDescriptorWrites.empty())
    {
        vkCmdPushDescriptorSetKHR(mCommandBuffer, bindingPoint, layout, 0, mDescriptorWrites.size(), mDescriptorWrites.data());
    }
}


bool
CommandBufferImpl::cmdBlitImage(Coral::Image* source, Coral::Image* dest)
{
    auto srcImpl = static_cast<Coral::Vulkan::ImageImpl*>(source);
    auto dstImpl = static_cast<Coral::Vulkan::ImageImpl*>(dest);

    // Transition the image layout of the source and destination image to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL and 
    // K_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.

    ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                        *srcImpl,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                        0,
                                        1,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                        *dstImpl,
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        0,
                                        1,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    // Blit the src image into the dst image
    VkImageBlit imageBlit{};
    imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.srcSubresource.layerCount = 1;
    imageBlit.srcSubresource.mipLevel   = 0;
    imageBlit.srcOffsets[1].x           = int32_t(source->width());
    imageBlit.srcOffsets[1].y           = int32_t(source->height());
    imageBlit.srcOffsets[1].z           = 1;

    imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBlit.dstSubresource.layerCount = 1;
    imageBlit.dstSubresource.mipLevel   = 0;
    imageBlit.dstOffsets[1].x           = int32_t(dest->width());
    imageBlit.dstOffsets[1].y           = int32_t(dest->height());
    imageBlit.dstOffsets[1].z           = 1;

    vkCmdBlitImage(mCommandBuffer, srcImpl->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   dstImpl->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

    // Finally, transition the image layouts back to the preferred

    ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                        *srcImpl,
                                        srcImpl->getPreferredImageLayout(),
                                        0,
                                        1,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    ImageImpl::cmdTransitionImageLayout(mCommandBuffer,
                                        *dstImpl,
                                        dstImpl->getPreferredImageLayout(),
                                        0,
                                        1,
                                        VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT,
                                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    return true;
}


VkCommandBuffer 
CommandBufferImpl::getVkCommandBuffer()
{
    return mCommandBuffer;
}


std::vector<std::shared_ptr<Coral::Buffer>>
CommandBufferImpl::getStagingBuffers()
{
    std::vector<std::shared_ptr<Coral::Buffer>> stagingBuffers;
    stagingBuffers.swap(mStagingBuffers);

    return stagingBuffers;
}