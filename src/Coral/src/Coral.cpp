#include <Coral/Coral.hpp>

#include "IFactory.hpp"

namespace Coral
{

Coral::Context*
createContext(const Coral::ContextConfig& config)
{
    return IFactory::get(config.graphicsAPI).createContext(config);
}


void
destroy(Coral::Context* context)
{
    IFactory::destroy(context);
}


Coral::Buffer*
createBuffer(Coral::Context* context, const Coral::BufferConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createBuffer(context, config);
}


void
destroy(Coral::Buffer* buffer)
{
    IFactory::destroy(buffer);
}


Coral::BufferView*
createBufferView(Coral::Context* context, const Coral::BufferViewConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createBufferView(context, config);
}


void
destroy(Coral::BufferView* bufferView)
{
    IFactory::destroy(bufferView);
}


Coral::Fence*
createFence(Coral::Context* context)
{
    return IFactory::get(context->graphicsAPI()).createFence(context);
}


void
destroy(Coral::Fence* fence)
{
    IFactory::destroy(fence);
}


Coral::Surface*
createSurface(Coral::Context* context, const Coral::SurfaceConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createSurface(context, config);
}


void
destroy(Coral::Surface* surface)
{
    IFactory::destroy(surface);
}


Coral::CommandBuffer*
createCommandBuffer(Coral::Context* context, const Coral::CommandBufferConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createCommandBuffer(context, config);
}


void
destroy(Coral::CommandBuffer* commandBuffer)
{
    IFactory::destroy(commandBuffer);
}


Coral::Sampler*
createSampler(Coral::Context* context, const Coral::SamplerConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createSampler(context, config);
}


void
destroy(Coral::Sampler* sampler)
{
    IFactory::destroy(sampler);
}



Coral::ShaderModule*
createShaderModule(Coral::Context* context, const Coral::ShaderModuleConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createShaderModule(context, config);
}


void
destroy(Coral::ShaderModule* shaderModule)
{
    IFactory::destroy(shaderModule);
}



Coral::Image*
createImage(Coral::Context* context, const Coral::ImageConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createImage(context, config);
}


void
destroy(Coral::Image* image)
{
    IFactory::destroy(image);
}


Coral::Semaphore*
createSemaphore(Coral::Context* context)
{
    return IFactory::get(context->graphicsAPI()).createSemaphore(context);
}


void
destroy(Coral::Semaphore* semaphore)
{
    IFactory::destroy(semaphore);
}


Coral::PipelineState*
createPipelineState(Coral::Context* context, const Coral::PipelineStateConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createPipelineState(context, config);
}


void
destroy(Coral::PipelineState* pipelineState)
{
    IFactory::destroy(pipelineState);
}


Coral::Framebuffer*
createFramebuffer(Coral::Context* context, const Coral::FramebufferConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createFramebuffer(context, config);
}


void
destroy(Coral::Framebuffer* framebuffer)
{
    IFactory::destroy(framebuffer);
}


Coral::DescriptorSet*
createDescriptorSet(Coral::Context* context, const Coral::DescriptorSetConfig& config)
{
    return IFactory::get(context->graphicsAPI()).createDescriptorSet(context, config);
}


void
destroy(Coral::DescriptorSet* descriptorSet)
{
    IFactory::destroy(descriptorSet);
}

} // namespace Coral
