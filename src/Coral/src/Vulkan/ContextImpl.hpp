#ifndef CORAL_VULKAN_CONTEXTIMPL_HPP
#define CORAL_VULKAN_CONTEXTIMPL_HPP

#include <Coral/Context.hpp>
#include <Coral/RAII.hpp>

#include "Vulkan.hpp"

#include "CommandQueueImpl.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <span>

namespace Coral
{
class BufferPool;
}

namespace Coral::Vulkan
{

class CommandQueueImpl;

class ContextImpl : public Coral::Context
{
public:

	static Coral::ContextPtr create(const ContextCreateConfig& config);

	bool init(const ContextCreateConfig& config);

	virtual ~ContextImpl();

	GraphicsAPI graphicsAPI() const override { return GraphicsAPI::VULKAN; }

	Coral::CommandQueue* getGraphicsQueue() override;

	Coral::CommandQueue* getComputeQueue() override;
									
	Coral::CommandQueue* getTransferQueue() override;

	std::expected<Coral::BufferPtr, Coral::BufferCreationError> createBuffer(const Coral::BufferCreateConfig& config) override;

	std::expected<Coral::BufferViewPtr, Coral::BufferViewCreationError> createBufferView(const Coral::BufferViewCreateConfig& config) override;

	std::expected<Coral::FencePtr, Coral::FenceCreationError> createFence()  override;

	std::expected<Coral::FramebufferPtr, Coral::FramebufferCreationError> createFramebuffer(const Coral::FramebufferCreateConfig& config) override;

	std::expected<Coral::ImagePtr, Coral::ImageCreationError> createImage(const Coral::ImageCreateConfig& config) override;

	std::expected<Coral::PipelineStatePtr, Coral::PipelineStateCreationError> createPipelineState(const Coral::PipelineStateCreateConfig& config) override;

	std::expected<Coral::SamplerPtr, Coral::SamplerCreationError> createSampler(const Coral::SamplerCreateConfig& config) override;

	std::expected<Coral::SemaphorePtr, Coral::SemaphoreCreationError> createSemaphore() override;

	std::expected<Coral::ShaderModulePtr, Coral::ShaderModuleCreationError> createShaderModule(const Coral::ShaderModuleCreateConfig& config) override;
	
	std::expected<Coral::SurfacePtr, Coral::SurfaceCreationError> createSurface(const Coral::SurfaceCreateConfig& config) override;

	VkInstance getVkInstance() { return mInstance; }

	VkDevice getVkDevice() { return mDevice; }

	VkPhysicalDevice getVkPhysicalDevice() { return mPhysicalDevice; }

	VmaAllocator getVmaAllocator();

	uint32_t getQueueFamilyIndex();

	/// Request a staging buffer from the staging buffer pool
	/**
	 * The staging buffer will have at least the requested buffer size. Staging buffers are returned to the to the pool
	 * after the using command buffer was executed.
	 */
	std::shared_ptr<Coral::Buffer> requestStagingBuffer(size_t bufferSize);

private:
	
	ContextImpl() = default;

	template<typename T, typename U, typename Config, typename CreateError>
	std::expected<std::unique_ptr<T, Deleter<T>>, CreateError> create(const Config& config)
	{
		auto obj = new U;

		if (auto error = obj->init(*this, config))
		{
			delete obj;
			return std::unexpected(*error);
		}

		return { std::unique_ptr<T, Deleter<T>>(obj) };
	}

	template<typename T, typename U, typename CreateError>
	std::expected<std::unique_ptr<T, Deleter<T>>, CreateError> create()
	{
		auto obj = new U;

		if (auto error = obj->init(*this))
		{
			delete obj;
			return std::unexpected(*error);
		}

		return { std::unique_ptr<T, Deleter<T>>(obj) };
	}


	template<typename T>
	void destroy(T* object)
	{
		delete object;
	}

	VkInstance mInstance{ VK_NULL_HANDLE };

	VkDebugUtilsMessengerEXT mDebugMessenger{ VK_NULL_HANDLE };

	VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };

	VkDevice mDevice{ VK_NULL_HANDLE };

	VmaAllocator mAllocator{ VK_NULL_HANDLE };

	VkCommandPool mCommandPool{ VK_NULL_HANDLE };

	uint32_t mQueueFamilyIndex{ 0 };

	std::shared_ptr<CommandQueueImpl> mTransferQueue;

	std::shared_ptr<CommandQueueImpl> mGraphicsQueue;

	std::shared_ptr<CommandQueueImpl> mComputeQueue;

	std::mutex mCommandPoolsProtection;

	std::unique_ptr<BufferPool> mStagingBufferPool;

	VkPhysicalDeviceProperties mProperties;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_CONTEXTIMPL_HPP