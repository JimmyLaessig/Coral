#include <Coral/Coral.hpp>

#include <Coral/BufferBase.hpp>
#include <Coral/BufferViewBase.hpp>
#include <Coral/CommandBufferBase.hpp>
#include <Coral/CommandQueueBase.hpp>
#include <Coral/FenceBase.hpp>
#include <Coral/FramebufferBase.hpp>
#include <Coral/ImageBase.hpp>
#include <Coral/PipelineStateBase.hpp>
#include <Coral/SamplerBase.hpp>
#include <Coral/SemaphoreBase.hpp>
#include <Coral/ShaderModuleBase.hpp>
#include <Coral/SurfaceBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>

namespace Coral
{

std::expected<Coral::ContextPtr, Coral::ContextCreationError>
createContext(const Coral::ContextCreateConfig& config)
{
    switch (config.graphicsAPI)
    {
        case GraphicsAPI::VULKAN:
            return Coral::ContextPtr{ Coral::Vulkan::ContextImpl::create(config) };
        default:
            return nullptr;
    }
}


void
destroy(Context* context)
{
    delete context;
}


void
destroy(Coral::Buffer* buffer)
{
    auto base = static_cast<Coral::BufferBase*>(buffer);
    base->context().destroy(base);
}


void
destroy(Coral::BufferView* bufferView)
{
    auto base = static_cast<Coral::BufferViewBase*>(bufferView);
    base->context().destroy(base);
}


void
destroy(Coral::CommandBuffer* commandBuffer)
{
    auto base = static_cast<Coral::CommandBufferBase*>(commandBuffer);
    base->commandQueue().destroyCommandBuffer(base);
}


void
destroy(Coral::Fence* fence)
{
    auto base = static_cast<Coral::FenceBase*>(fence);
    base->context().destroy(base);
}


void
destroy(Coral::Framebuffer* framebuffer)
{
    auto base = static_cast<Coral::FramebufferBase*>(framebuffer);
    base->context().destroy(base);
}


void
destroy(Coral::Image* image)
{
    auto base = static_cast<Coral::ImageBase*>(image);
    base->context().destroy(base);
}


void
destroy(Coral::PipelineState* pipelineState)
{
    auto base = static_cast<Coral::PipelineStateBase*>(pipelineState);
    base->context().destroy(base);
}


void
destroy(Coral::Sampler* sampler)
{
    auto base = static_cast<Coral::SamplerBase*>(sampler);
    base->context().destroy(base);
}

void
destroy(Coral::Semaphore* semaphore)
{
    auto base = static_cast<Coral::SemaphoreBase*>(semaphore);
    base->context().destroy(base);
}


void
destroy(Coral::ShaderModule* shaderModule)
{
    auto base = static_cast<Coral::ShaderModuleBase*>(shaderModule);
    base->context().destroy(base);
}

void
destroy(Coral::Surface* surface)
{
    auto base = static_cast<Coral::SurfaceBase*>(surface);
    base->context().destroy(base);
}


    
} // namespace Coral
