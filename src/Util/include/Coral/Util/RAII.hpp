#include <memory>

#include <Coral/Coral.h>

namespace Coral
{

auto destroyContext = [](CoContext context)
{
    coDestroyContext(context);
};

using ContextPtr = std::unique_ptr<CoContext_T, decltype(destroyContext)>;


auto destroySwapchain = [](CoSwapchain swapchain)
{
    coDestroySwapchain(swapchain);
};

using SwapchainPtr = std::unique_ptr<CoSwapchain_T, decltype(destroySwapchain)>;


auto destroyBuffer = [](CoBuffer buffer)
{
    coDestroyBuffer(buffer);
};

using BufferPtr = std::unique_ptr<CoBuffer_T, decltype(destroyBuffer)>;


auto destroyCommandBuffer = [](CoCommandBuffer buffer)
{
    coDestroyCommandBuffer(buffer);
};

using CommandBufferPtr = std::unique_ptr<CoCommandBuffer_T, decltype(destroyCommandBuffer)>;


auto destroyFence = [](CoFence fence)
{
    coDestroyFence(fence);
};

using FencePtr = std::unique_ptr<CoFence_T, decltype(destroyFence)>;


auto destroySemaphore = [](CoSemaphore semaphore)
{
    coDestroySemaphore(semaphore);
};

using SemaphorePtr = std::unique_ptr<CoSemaphore_T, decltype(destroySemaphore)>;


auto destroyImage = [](CoImage image)
{
    coDestroyImage(image);
};

using ImagePtr = std::unique_ptr<CoImage_T, decltype(destroyImage)>;


auto destroySampler = [](CoSampler sampler)
{
    coDestroySampler(sampler);
};

using SamplerPtr = std::unique_ptr<CoSampler_T, decltype(destroySampler)>;


auto destroyShaderModule = [](CoShaderModule shaderModule)
{
    coDestroyShaderModule(shaderModule);
};

using ShaderModulePtr = std::unique_ptr<CoShaderModule_T, decltype(destroyShaderModule)>;


auto destroyPipelineState = [](CoPipelineState pipelineState)
{
    coDestroyPipelineState(pipelineState);
};

using PipelineStatePtr = std::unique_ptr<CoPipelineState_T, decltype(destroyPipelineState)>;

} // namespace Coral
