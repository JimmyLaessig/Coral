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

namespace Coral
{

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

	/// Create a new Swapchain object
	virtual std::expected<Coral::SwapchainPtr, Coral::SwapchainCreationError> createSwapchain(const Coral::SwapchainCreateConfig& config) = 0;
};


/// Create a new context object
CORAL_API std::expected<Coral::ContextPtr, Coral::ContextCreationError> createContext(const ContextCreateConfig& config);


} // namespace Coral

#endif // !CORAL_CONTEXT_HPP