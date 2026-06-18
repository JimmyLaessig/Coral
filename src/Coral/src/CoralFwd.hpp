#ifndef CORAL_CORALFWD_HPP
#define CORAL_CORALFWD_HPP

#include <memory>

namespace Coral
{

class Buffer;
class CommandBuffer;
class CommandQueue;
class Context;
class Fence;
class Framebuffer;
class Image;
class PipelineState;
class Sampler;
class Semaphore;
class ShaderModule;
class Swapchain;

using BufferPtr        = std::shared_ptr<Buffer>;
using CommandBufferPtr = std::shared_ptr<CommandBuffer>;
using ContextPtr       = std::shared_ptr<Context>;
using FencePtr         = std::shared_ptr<Fence>;
using FramebufferPtr   = std::shared_ptr<Framebuffer>;
using ImagePtr         = std::shared_ptr<Image>;
using PipelineStatePtr = std::shared_ptr<PipelineState>;
using SamplerPtr       = std::shared_ptr<Sampler>;
using SemaphorePtr     = std::shared_ptr<Semaphore>;
using ShaderModulePtr  = std::shared_ptr<ShaderModule>;
using SwapchainPtr     = std::shared_ptr<Swapchain>;

} // namespace Coral

#endif // !CORAL_CORALFWD_HPP
