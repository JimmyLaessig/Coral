#ifndef CORAL_CONTEXT_HPP
#define CORAL_CONTEXT_HPP

#include <Coral/System.hpp>
#include <Coral/CoralFwd.hpp>

#include <Coral/Buffer.hpp>
#include <Coral/CommandBuffer.hpp>
#include <Coral/CommandQueue.hpp>
#include <Coral/Fence.hpp>
#include <Coral/Framebuffer.hpp>
#include <Coral/Image.hpp>
#include <Coral/PipelineState.hpp>
#include <Coral/Sampler.hpp>
#include <Coral/Semaphore.hpp>
#include <Coral/ShaderModule.hpp>
#include <Coral/Swapchain.hpp>

#include <expected>
#include <string_view>

#include <unordered_map>

namespace Coral
{

class Context;
class Buffer;
class CommandBuffer;
class CommandQueue;
class Fence;
class Framebuffer;
class Image;
class PipelineState;
class Sampler;
class Semaphore;
class Swapchain;
class ShaderModule;

using ContextPtr       = std::shared_ptr<Context>;
using BufferPtr        = std::shared_ptr<Buffer>;
using CommandBufferPtr = std::shared_ptr<CommandBuffer>;
using FencePtr         = std::shared_ptr<Fence>;
using FramebufferPtr   = std::shared_ptr<Framebuffer>;
using ImagePtr         = std::shared_ptr<Image>;
using PipelineStatePtr = std::shared_ptr<PipelineState>;
using SamplerPtr       = std::shared_ptr<Sampler>;
using SemaphorePtr     = std::shared_ptr<Semaphore>;
using SwapchainPtr     = std::shared_ptr<Swapchain>;
using ShaderModulePtr  = std::shared_ptr<ShaderModule>;


class CORAL_API Context
{
public:

	using CreateConfig = CoContextCreateConfig;

	enum CreateError
	{
		UNSUPPORTED_GRAPHICS_API = CO_ERROR_UNSUPPORTED_GRAPHICS_API,
		INTERNAL_ERROR = CO_ERROR_INTERNAL,
	};

	static std::expected<std::shared_ptr<Context>, CreateError> create(const CreateConfig& config);



	virtual ~Context() = default;

	/// Get the Graphics API
	virtual CoGraphicsAPI graphicsAPI() const = 0;

	/// Get the preferred CommandQueue for graphics commands
	virtual Coral::CommandQueue* getGraphicsQueue() = 0;

	/// Get the preferred CommandQueue for compute commands
	virtual Coral::CommandQueue* getComputeQueue() = 0;

	/// Get the preferred CommandQueue for transfer commands
	virtual Coral::CommandQueue* getTransferQueue() = 0;

	/// Create a new Buffer object
    virtual std::expected<Coral::BufferPtr, Coral::Buffer::CreateError> createBuffer(const Coral::Buffer::CreateConfig& config) = 0;

	/// Create a new Fence object
	virtual std::expected<Coral::FencePtr, Coral::Fence::CreateError> createFence(const Coral::Fence::CreateConfig& config) = 0;

	/// Create a new Framebuffer object
	virtual std::expected<Coral::FramebufferPtr, Coral::Framebuffer::CreateError> createFramebuffer(const Coral::Framebuffer::CreateConfig& config) = 0;

	/// Create a new Image object
	virtual std::expected<Coral::ImagePtr, Coral::Image::CreateError> createImage(const Coral::Image::CreateConfig& config) = 0;

	/// Create a new PipelineState object
	virtual std::expected<Coral::PipelineStatePtr, Coral::PipelineState::CreateError> createPipelineState(const Coral::PipelineState::CreateConfig& config) = 0;

	/// Create a new Sampler object
	virtual std::expected<Coral::SamplerPtr, Coral::Sampler::CreateError> createSampler(const Coral::Sampler::CreateConfig& config) = 0;

	/// Create a new Semaphore object
	virtual std::expected<Coral::SemaphorePtr, Coral::Semaphore::CreateError> createSemaphore(const Coral::Semaphore::CreateConfig& config) = 0;

	/// Create a new ShaderModule object
    virtual std::expected<Coral::ShaderModulePtr, Coral::ShaderModule::CreateError> createShaderModule(const Coral::ShaderModule::CreateConfig& config) = 0;

	/// Create a new Swapchain object
	virtual std::expected<Coral::SwapchainPtr, Coral::Swapchain::CreateError> createSwapchain(const Coral::Swapchain::CreateConfig& config) = 0;
};

} // namespace Coral

struct CoContext_T
{
	Coral::ContextPtr impl;

	std::unordered_map<Coral::CommandQueue*, std::unique_ptr<CoCommandQueue_T>> m_commandQueues;
};

#endif // !CORAL_CONTEXT_HPP