#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandQueue.hpp"
#include "Fence.hpp"
#include "Image.hpp"
#include "PipelineState.hpp"
#include "Sampler.hpp"
#include "Semaphore.hpp"

#include <ranges>

using namespace Coral;

CoResult
coCommandQueueCreateCommandBuffer(CoCommandQueue queue, const CoCommandBufferCreateConfig* pConfig, CoCommandBuffer* pCommandBuffer)
{
    if (auto impl = queue->impl->createCommandBuffer(*pConfig))
    {
        *pCommandBuffer = new CoCommandBuffer_T{ impl.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroyCommandBuffer(CoCommandBuffer commandBuffer)
{
    delete commandBuffer;
}


CoResult
coCommandBufferBegin(CoCommandBuffer commandBuffer)
{
    return commandBuffer->impl->begin() ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferEnd(CoCommandBuffer commandBuffer)
{
    return commandBuffer->impl->end() ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferBeginRenderPass(CoCommandBuffer commandBuffer, const CoBeginRenderPassInfo* beginInfo)
{
    Coral::BeginRenderPassInfo info{};
    info.framebuffer = beginInfo->framebuffer->impl.get();
    for (const auto& attachment : std::span(beginInfo->pClearColors, beginInfo->clearColorsCount))
    {
        ClearColor clearColor
        {
            attachment.clearOp,
            { attachment.color[0], attachment.color[1], attachment.color[2], attachment.color[3] }
        };
        
        if (!info.clearColor.emplace(attachment.attachment, clearColor).second)
        {
            return CO_FAILED;
        }
    }
    if (beginInfo->clearDepthStencil)
    {
        info.clearDepth = *beginInfo->clearDepthStencil;
    }
    return commandBuffer->impl->cmdBeginRenderPass(info) ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferEndRenderPass(CoCommandBuffer commandBuffer)
{
    return commandBuffer->impl->cmdEndRenderPass() ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferUpdateBufferData(CoCommandBuffer commandBuffer, const CoUpdateBufferDataInfo* updateInfo)
{
    Coral::UpdateBufferDataInfo info{};
    info.buffer = updateInfo->buffer->impl;
    info.data   = std::as_bytes(std::span(updateInfo->pData, updateInfo->dataCount));
    info.offset = updateInfo->offset;
    return commandBuffer->impl->cmdUpdateBufferData(info) ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferUpdateImageData(CoCommandBuffer commandBuffer, const CoUpdateImageDataInfo* updateInfo)
{
    Coral::UpdateImageDataInfo info{};
    info.image = updateInfo->image->impl;
    info.data  = std::as_bytes(std::span(updateInfo->pData, updateInfo->dataCount));
    return commandBuffer->impl->cmdUpdateImageData(info) ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferGenerateMipMaps(CoCommandBuffer commandBuffer, CoImage image)
{
    return commandBuffer->impl->cmdGenerateMipMaps(image->impl) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBlitImage(CoCommandBuffer commandBuffer, CoImage source, CoImage dest)
{
    return CO_FAILED;
}


CoResult
coCommandBufferBindVertexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, uint32_t location, size_t offset, size_t stride)
{
    return commandBuffer->impl->cmdBindVertexBuffer(buffer->impl, location, offset, stride) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBindIndexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, CoIndexFormat format, size_t offset)
{
    return commandBuffer->impl->cmdBindIndexBuffer(buffer->impl, format, offset) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBindPipeline(CoCommandBuffer commandBuffer, CoPipelineState pipeline)
{
    return commandBuffer->impl->cmdBindPipeline(pipeline->impl) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferSetViewport(CoCommandBuffer commandBuffer, const CoViewportInfo* info)
{
    return commandBuffer->impl->cmdSetViewport(*info) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBindDescriptor(CoCommandBuffer commandBuffer, const CoDescriptor* pDescriptor, uint32_t binding)
{
    auto impl = commandBuffer->impl;

    switch (pDescriptor->type)
    {
        case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            impl->cmdBindDescriptor(pDescriptor->buffer->impl, binding);
            return CO_SUCCESS;
        case CO_DESCRIPTOR_TYPE_IMAGE:
            impl->cmdBindDescriptor(pDescriptor->image->impl, binding);
            return CO_SUCCESS;
        case CO_DESCRIPTOR_TYPE_SAMPLER:
            impl->cmdBindDescriptor(pDescriptor->sampler->impl, binding);
            return CO_SUCCESS;
        case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            impl->cmdBindDescriptor(pDescriptor->combinedImageSampler.image->impl, pDescriptor->combinedImageSampler.sampler->impl, binding);
            return CO_SUCCESS;
        default:
            return CO_FAILED;
    }

    return CO_SUCCESS;
}


CoResult
coCommandBufferDraw(CoCommandBuffer commandBuffer, const CoDrawInfo* info)
{
    return commandBuffer->impl->cmdDraw(*info) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferDrawIndirect(CoCommandBuffer commandBuffer, CoBuffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
{
    return commandBuffer->impl->cmdDrawIndirect(buffer->impl, offset, drawCount, stride) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferDrawIndexed(CoCommandBuffer commandBuffer, const CoDrawIndexedInfo* info)
{
    return commandBuffer->impl->cmdDrawIndexed(*info) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferDrawIndexedIndirect(CoCommandBuffer commandBuffer, CoBuffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
{
    return commandBuffer->impl->cmdDrawIndexedIndirect(buffer->impl, offset, drawCount, stride) ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandQueueSubmit(CoCommandQueue queue, const CoCommandBufferSubmitInfo* submitInfo, CoFence fence)
{
    Coral::CommandBufferSubmitInfo info{};

    auto commandBuffers = std::span(submitInfo->pCommandBuffers, submitInfo->commandBufferCount)
        | std::views::transform([](auto cb) { return cb->impl; })
        | std::ranges::to<std::vector>();

    auto waitSemaphores = std::span(submitInfo->pWaitSemaphores, submitInfo->waitSemaphoreCount)
        | std::views::transform([](auto cb) { return cb->impl; }) 
        | std::ranges::to<std::vector>();

    auto signalSemaphores = std::span(submitInfo->pSignalSemaphores, submitInfo->signalSemaphoreCount)
        | std::views::transform([](auto cb) { return cb->impl; }) 
        | std::ranges::to<std::vector>();

    info.commandBuffers   = commandBuffers;
    info.waitSemaphores   = waitSemaphores;
    info.signalSemaphores = signalSemaphores;

    return queue->impl->submit(info, fence ? fence->impl : nullptr) ? CO_SUCCESS : CO_FAILED;
}
