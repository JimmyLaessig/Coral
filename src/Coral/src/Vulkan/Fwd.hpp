#ifndef CORAL_VULKAN_FWD_HPP
#define CORAL_VULKAN_FWD_HPP

#include <memory>

namespace Coral::Vulkan
{
class Resource;
class BufferImpl;
class CommandBufferImpl;
class CommandQueueImpl;
class ContextImpl;
class DescriptorSetImpl;
class FenceImpl;
class FramebufferImpl;
class ImageImpl;
class GraphicsPipelineImpl;
class SamplerImpl;
class SemaphoreImpl;
class ShaderModuleImpl;
class SwapchainImpl;

using ResourcePtr          = std::shared_ptr<Resource>;
using BufferImplPtr        = std::shared_ptr<BufferImpl>;
using CommandBufferImplPtr = std::shared_ptr<CommandBufferImpl>;
using ContextImplPtr       = std::shared_ptr<ContextImpl>;
using DescriptorSetImplPtr = std::shared_ptr<DescriptorSetImpl>;
using FenceImplPtr         = std::shared_ptr<FenceImpl>;
using FramebufferImplPtr   = std::shared_ptr<FramebufferImpl>;
using ImageImplPtr         = std::shared_ptr<ImageImpl>;
using GraphicsPipelineImplPtr = std::shared_ptr<GraphicsPipelineImpl>;
using SamplerImplPtr       = std::shared_ptr<SamplerImpl>;
using SemaphoreImplPtr     = std::shared_ptr<SemaphoreImpl>;
using ShaderModuleImplPtr  = std::shared_ptr<ShaderModuleImpl>;
using SwapchainImplPtr     = std::shared_ptr<SwapchainImpl>;

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FWD_HPP
