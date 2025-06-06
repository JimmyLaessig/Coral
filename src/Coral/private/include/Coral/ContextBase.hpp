#ifndef CORAL_CONTEXTBASE_HPP
#define CORAL_CONTEXTBASE_HPP

#include <Coral/Context.hpp>


namespace Coral
{

class BufferBase;
class BufferViewBase;
class FenceBase;
class FramebufferBase;
class ImageBase;
class PipelineStateBase;
class SamplerBase;
class SemaphoreBase;
class ShaderModuleBase;
class SurfaceBase;


class ContextBase : public Coral::Context
{
public:

    virtual void destroy(Coral::BufferBase* buffer) = 0;

    virtual void destroy(Coral::BufferViewBase* bufferView) = 0;

    virtual void destroy(Coral::FenceBase* fence) = 0;

    virtual void destroy(Coral::FramebufferBase* framebuffer) = 0;

    virtual void destroy(Coral::ImageBase* image) = 0;

    virtual void destroy(Coral::PipelineStateBase* pipelineState) = 0;

    virtual void destroy(Coral::SamplerBase* sampler) = 0;

    virtual void destroy(Coral::SemaphoreBase* semaphore) = 0;

    virtual void destroy(Coral::ShaderModuleBase* shaderModule) = 0;

    virtual void destroy(Coral::SurfaceBase* surface) = 0;
};

} // namespace

#endif // !CORAL_CONTEXTBASE_HPP
