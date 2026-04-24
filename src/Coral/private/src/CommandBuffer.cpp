#include <Coral/CommandBuffer.h>
#include <Coral/CommandBuffer.hpp>
#include <Coral/CommandQueue.hpp>

#include <Coral/Sampler.hpp>

#include <Coral/PipelineState.hpp>
#include <Coral/Buffer.hpp>
#include <Coral/Image.hpp>
#include <Coral/Fence.hpp>
#include <Coral/Semaphore.hpp>
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
    info.buffer = updateInfo->buffer->impl.get();
    info.data   = std::as_bytes(std::span(updateInfo->pData, updateInfo->dataCount));
    info.offset = updateInfo->offset;
    return commandBuffer->impl->cmdUpdateBufferData(info) ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferUpdateImageData(CoCommandBuffer commandBuffer, const CoUpdateImageDataInfo* updateInfo)
{
    Coral::UpdateImageDataInfo info{};
    info.image = updateInfo->image->impl.get();
    info.data  = std::as_bytes(std::span(updateInfo->pData, updateInfo->dataCount));
    return commandBuffer->impl->cmdUpdateImageData(info) ? CO_SUCCESS : CO_FAILED;
}


CoResult 
coCommandBufferGenerateMipMaps(CoCommandBuffer commandBuffer, CoImage image)
{
    return commandBuffer->impl->cmdGenerateMipMaps(image->impl.get()) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBlitImage(CoCommandBuffer commandBuffer, CoImage source, CoImage dest)
{
    return CO_FAILED;
}


CoResult
coCommandBufferBindVertexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, uint32_t location, size_t offset, size_t stride)
{
    return commandBuffer->impl->cmdBindVertexBuffer(buffer->impl.get(), location, offset, stride) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBindIndexBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, CoIndexFormat format, size_t offset)
{
    return commandBuffer->impl->cmdBindIndexBuffer(buffer->impl.get(), format, offset) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBindPipeline(CoCommandBuffer commandBuffer, CoPipelineState pipeline)
{
    return commandBuffer->impl->cmdBindPipeline(pipeline->impl.get()) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferSetViewport(CoCommandBuffer commandBuffer, const CoViewportInfo* info)
{
    return commandBuffer->impl->cmdSetViewport(*info) ? CO_SUCCESS : CO_FAILED;
}


CoResult
coCommandBufferBindUniformBuffer(CoCommandBuffer commandBuffer, CoBuffer buffer, uint32_t binding)
{
    commandBuffer->impl->cmdBindDescriptor(buffer->impl.get(), binding);
    return CO_SUCCESS;
}


CoResult
coCommandBufferBindImage(CoCommandBuffer commandBuffer, CoImage image, uint32_t binding)
{
    commandBuffer->impl->cmdBindDescriptor(image->impl.get(), binding);
    return CO_SUCCESS;
}


CoResult
coCommandBufferBindSampler(CoCommandBuffer commandBuffer, CoSampler sampler, uint32_t binding)
{
    commandBuffer->impl->cmdBindDescriptor(sampler->impl.get(), binding);
    return CO_SUCCESS;
}


CoResult
coCommandBufferDrawIndexed(CoCommandBuffer commandBuffer, uint32_t firstIndex, uint32_t indexCount)
{
    return commandBuffer->impl->cmdDrawIndexed({ indexCount, firstIndex }) ? CO_SUCCESS : CO_FAILED;
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