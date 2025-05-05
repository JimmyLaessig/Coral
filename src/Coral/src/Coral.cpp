#include <Coral/Coral.hpp>

#include "BufferBase.hpp"
#include "BufferViewBase.hpp"
#include "CommandBufferBase.hpp"
#include "CommandQueueBase.hpp"
#include "FenceBase.hpp"
#include "FramebufferBase.hpp"
#include "ImageBase.hpp"
#include "PipelineStateBase.hpp"
#include "SamplerBase.hpp"
#include "SemaphoreBase.hpp"
#include "ShaderModuleBase.hpp"
#include "SurfaceBase.hpp"

#include "Vulkan/ContextImpl.hpp"

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
