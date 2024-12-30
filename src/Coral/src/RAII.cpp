#include <Coral/RAII.hpp>


namespace Coral::RAII
{

ImagePtr
createImage(Context* context, const ImageConfig& config)
{
	return ImagePtr(Coral::createImage(context, config));
}


BufferPtr
createBuffer(Context* context, const BufferConfig& config)
{
	return BufferPtr(Coral::createBuffer(context, config));
}


BufferViewPtr
createBufferView(Context* context, const BufferViewConfig& config)
{
	return BufferViewPtr(Coral::createBufferView(context, config));
}


CommandBufferPtr
createCommandBuffer(Context* context, const CommandBufferConfig& config)
{
	return CommandBufferPtr(Coral::createCommandBuffer(context, config));
}


ContextPtr
createContext(const ContextConfig& config)
{
	return ContextPtr(Coral::createContext(config));
}


SamplerPtr
createSampler(Context* context, const SamplerConfig& config)
{
	return SamplerPtr(Coral::createSampler(context, config));
}


SurfacePtr
createSurface(Context* context, const SurfaceConfig& config)
{
	return SurfacePtr(Coral::createSurface(context, config));
}


FencePtr
createFence(Context* context)
{
	return FencePtr(Coral::createFence(context));
}


PipelineStatePtr
createPipelineState(Context* context, const PipelineStateConfig& config)
{
	return PipelineStatePtr(Coral::createPipelineState(context, config));
}


ShaderModulePtr
createShaderModule(Context* context, const ShaderModuleConfig& config)
{
	return ShaderModulePtr(Coral::createShaderModule(context, config));
}

SemaphorePtr
createSemaphore(Context* context)
{
	return SemaphorePtr(Coral::createSemaphore(context));
}


DescriptorSetPtr
createDescriptorSet(Context* context, const DescriptorSetConfig& config)
{
	return DescriptorSetPtr(Coral::createDescriptorSet(context, config));
}


FramebufferPtr
createFramebuffer(Context* context, const FramebufferConfig& config)
{
	return FramebufferPtr(Coral::createFramebuffer(context, config));
}

} // namespace Coral::RAII

