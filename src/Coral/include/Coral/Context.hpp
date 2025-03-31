#ifndef CORAL_CONTEXT_HPP
#define CORAL_CONTEXT_HPP

#include <Coral/System.hpp>

#include <Coral/Buffer.hpp>
#include <Coral/BufferView.hpp>

#include <Coral/CommandBuffer.hpp>
#include <Coral/Fence.hpp>
#include <Coral/Framebuffer.hpp>
#include <Coral/Image.hpp>
#include <Coral/PipelineState.hpp>
#include <Coral/Sampler.hpp>
#include <Coral/Semaphore.hpp>
#include <Coral/ShaderModule.hpp>
#include <Coral/Surface.hpp>

#include <expected>

namespace Coral
{
class Context;
class CommandQueue;

/// Destroy the Buffer object
CORAL_API void destroy(Buffer* buffer);

/// Destroy the BufferView object
CORAL_API void destroy(BufferView* bufferView);

/// Destroy the Context object
CORAL_API void destroy(Context* context);

/// Destroy the Fence object
CORAL_API void destroy(Fence* fence);

/// Destroy the Framebuffer object
CORAL_API void destroy(Framebuffer* framebuffer);

/// Destroy the CommandBuffer object
CORAL_API void destroy(CommandBuffer* commandBuffer);

/// Destroy the Image object
CORAL_API void destroy(Image* image);

/// Destroy the PipelineState object
CORAL_API void destroy(PipelineState* pipelineState);

/// Destroy the Sampler object
CORAL_API void destroy(Sampler* sampler);

/// Destroy the Semaphore object
CORAL_API void destroy(Semaphore* semaphore);

/// Destroy the ShaderModule object
CORAL_API void destroy(ShaderModule* shaderModule);

/// Destroy the Surface object
CORAL_API void destroy(Surface* surface);


template<typename T>
struct CORAL_API Deleter
{
	void operator()(T* t)
	{
		Coral::destroy(t);
	}
};

using BufferPtr        = std::unique_ptr<Buffer, Deleter<Buffer>>;
using BufferViewPtr    = std::unique_ptr<BufferView, Deleter<BufferView>>;
using CommandBufferPtr = std::unique_ptr<CommandBuffer, Deleter<CommandBuffer>>;
using FencePtr         = std::unique_ptr<Fence, Deleter<Fence>>;
using FramebufferPtr   = std::unique_ptr<Framebuffer, Deleter<Framebuffer>>;
using ImagePtr		   = std::unique_ptr<Image, Deleter<Image>>;
using PipelineStatePtr = std::unique_ptr<PipelineState, Deleter<PipelineState>>;
using SamplerPtr	   = std::unique_ptr<Sampler, Deleter<Sampler>>;
using SemaphorePtr     = std::unique_ptr<Semaphore, Deleter<Semaphore>>;
using ShaderModulePtr  = std::unique_ptr<ShaderModule, Deleter<ShaderModule>>;
using SurfacePtr	   = std::unique_ptr<Surface, Deleter<Surface>>;


enum class GraphicsAPI
{
	VULKAN
};

enum class ContextCreationError
{
	INTERNAL_ERROR
};

struct ContextCreateConfig
{
	GraphicsAPI graphicsAPI{ GraphicsAPI::VULKAN };

	std::string_view applicationName{ "Coral" };
};

class CORAL_API Context
{
public:

	virtual ~Context() = default;

	/// Get the Graphics API
	virtual GraphicsAPI graphicsAPI() const = 0;

	/// Get the preferred CommandQueue for graphics commands
	virtual Coral::CommandQueue* getGraphicsQueue() = 0;

	/// Get the preferred CommandQueue for compute commands
	virtual Coral::CommandQueue* getComputeQueue() = 0;

	/// Get the preferred CommandQueue for transfer commands
	virtual Coral::CommandQueue* getTransferQueue() = 0;

	/// Create a new Buffer object
    virtual std::expected<Coral::BufferPtr, Coral::BufferCreationError> createBuffer(const Coral::BufferCreateConfig& config) = 0;

	/// Create a new BufferView object
    virtual std::expected<Coral::BufferViewPtr, Coral::BufferViewCreationError> createBufferView(const Coral::BufferViewCreateConfig& config) = 0;

	/// Create a new Fence object
	virtual std::expected<Coral::FencePtr, Coral::FenceCreationError> createFence() = 0;

	/// Create a new Framebuffer object
	virtual std::expected<Coral::FramebufferPtr, Coral::FramebufferCreationError> createFramebuffer(const Coral::FramebufferCreateConfig& config) = 0;

	/// Create a new Image object
	virtual std::expected<Coral::ImagePtr, Coral::ImageCreationError> createImage(const Coral::ImageCreateConfig& config) = 0;

	/// Create a new PipelineState object
	virtual std::expected<Coral::PipelineStatePtr, Coral::PipelineStateCreationError> createPipelineState(const Coral::PipelineStateCreateConfig& config) = 0;

	/// Create a new Sampler object
	virtual std::expected<Coral::SamplerPtr, Coral::SamplerCreationError> createSampler(const Coral::SamplerCreateConfig& config) = 0;

	/// Create a new Semaphore object
	virtual std::expected<Coral::SemaphorePtr, Coral::SemaphoreCreationError> createSemaphore() = 0;

	/// Create a new ShaderModule object
    virtual std::expected<Coral::ShaderModulePtr, Coral::ShaderModuleCreationError> createShaderModule(const Coral::ShaderModuleCreateConfig& config) = 0;

	/// Create a new Surface object
	virtual std::expected<Coral::SurfacePtr, Coral::SurfaceCreationError> createSurface(const Coral::SurfaceCreateConfig& config) = 0;
};

using ContextPtr = std::unique_ptr<Context, Deleter<Context>>;

/// Create a new context object
CORAL_API std::expected<Coral::ContextPtr, Coral::ContextCreationError> createContext(const ContextCreateConfig& config);

} // namespace Coral

#endif // !CORAL_CONTEXT_HPP