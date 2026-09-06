#ifndef CORAL_UTIL_RAII_HPP
#define CORAL_UTIL_RAII_HPP

#include <memory>

#include <Coral/Coral.h>

namespace Coral
{

inline auto destroyContext = [](CoContext context)
{
    coDestroyContext(context);
};

using ContextPtr = std::unique_ptr<CoContext_T, decltype(destroyContext)>;


inline auto destroySwapchain = [](CoSwapchain swapchain)
{
    coDestroySwapchain(swapchain);
};

using SwapchainPtr = std::unique_ptr<CoSwapchain_T, decltype(destroySwapchain)>;


inline auto destroyBuffer = [](CoBuffer buffer)
{
    coDestroyBuffer(buffer);
};

using BufferPtr = std::unique_ptr<CoBuffer_T, decltype(destroyBuffer)>;

inline auto destroyCommandBuffer = [](CoCommandBuffer buffer)
{
    coDestroyCommandBuffer(buffer);
};

using CommandBufferPtr = std::unique_ptr<CoCommandBuffer_T, decltype(destroyCommandBuffer)>;

inline auto destroyDescriptorSet = [](CoDescriptorSet set)
{
    coDestroyDescriptorSet(set);
};

using DescriptorSetPtr = std::unique_ptr<CoDescriptorSet_T, decltype(destroyDescriptorSet)>;

inline auto destroyFence = [](CoFence fence)
{
    coDestroyFence(fence);
};

using FencePtr = std::unique_ptr<CoFence_T, decltype(destroyFence)>;


inline auto destroySemaphore = [](CoSemaphore semaphore)
{
    coDestroySemaphore(semaphore);
};

using SemaphorePtr = std::unique_ptr<CoSemaphore_T, decltype(destroySemaphore)>;


inline auto destroyImage = [](CoImage image)
{
    coDestroyImage(image);
};

using ImagePtr = std::unique_ptr<CoImage_T, decltype(destroyImage)>;


inline auto destroySampler = [](CoSampler sampler)
{
    coDestroySampler(sampler);
};

using SamplerPtr = std::unique_ptr<CoSampler_T, decltype(destroySampler)>;


inline auto destroyShaderModule = [](CoShaderModule shaderModule)
{
    coDestroyShaderModule(shaderModule);
};

using ShaderModulePtr = std::unique_ptr<CoShaderModule_T, decltype(destroyShaderModule)>;


inline auto destroyGraphicsPipeline = [](CoGraphicsPipeline GraphicsPipeline)
{
    coDestroyGraphicsPipeline(GraphicsPipeline);
};

using GraphicsPipelinePtr = std::unique_ptr<CoGraphicsPipeline_T, decltype(destroyGraphicsPipeline)>;

} // namespace Coral

#endif // !CORAL_UTIL_RAII_HPP
