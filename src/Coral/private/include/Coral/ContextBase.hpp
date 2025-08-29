#ifndef CORAL_CONTEXTBASE_HPP
#define CORAL_CONTEXTBASE_HPP

#include <Coral/Context.hpp>


namespace Coral
{

class ContextBase : public Coral::Context
{
public:

    virtual void destroy(Coral::Buffer* buffer) = 0;

    virtual void destroy(Coral::Fence* fence) = 0;

    virtual void destroy(Coral::Framebuffer* framebuffer) = 0;

    virtual void destroy(Coral::Image* image) = 0;

    virtual void destroy(Coral::PipelineState* pipelineState) = 0;

    virtual void destroy(Coral::Sampler* sampler) = 0;

    virtual void destroy(Coral::Semaphore* semaphore) = 0;

    virtual void destroy(Coral::ShaderModule* shaderModule) = 0;

    virtual void destroy(Coral::Swapchain* surface) = 0;
};

} // namespace

#endif // !CORAL_CONTEXTBASE_HPP
