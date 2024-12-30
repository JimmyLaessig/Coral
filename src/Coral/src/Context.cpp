#include <Coral/Context.hpp>

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


void destroy(Context* context)
{
    delete context;
}


void
destroy(Coral::Buffer* buffer)
{
    delete buffer;
}


void
destroy(Coral::BufferView* bufferView)
{
    delete bufferView;
}


void
destroy(Coral::Fence* fence)
{
    delete fence;
}


void
destroy(Coral::Surface* surface)
{
    delete surface;
}


void
destroy(Coral::CommandBuffer* commandBuffer)
{
    delete commandBuffer;
}


void
destroy(Coral::Sampler* sampler)
{
    delete sampler;
}


void
destroy(Coral::ShaderModule* shaderModule)
{
    delete shaderModule;
}


void
destroy(Coral::Image* image)
{
    delete image;
}


void
destroy(Coral::Semaphore* semaphore)
{
    delete semaphore;
}


void
destroy(Coral::PipelineState* pipelineState)
{
    delete pipelineState;
}


void
destroy(Coral::Framebuffer* framebuffer)
{
    delete framebuffer;
}


void
destroy(Coral::DescriptorSet* descriptorSet)
{
    delete descriptorSet;
}

} // namespace Coral
