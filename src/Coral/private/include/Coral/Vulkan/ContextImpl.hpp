#ifndef CORAL_VULKAN_CONTEXTIMPL_HPP
#define CORAL_VULKAN_CONTEXTIMPL_HPP

#include <Coral/ContextBase.hpp>

#include <Coral/Vulkan/Vulkan.hpp>

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

class ContextImpl : public Coral::ContextBase
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
	
	std::expected<Coral::SwapchainPtr, Coral::SwapchainCreationError> createSwapchain(const Coral::SwapchainCreateConfig& config) override;

	void destroy(Coral::BufferBase* buffer) override;

	void destroy(Coral::BufferViewBase* bufferView) override;

	void destroy(Coral::FenceBase* fence) override;

	void destroy(Coral::FramebufferBase* framebuffer) override;

	void destroy(Coral::ImageBase* image) override;

	void destroy(Coral::PipelineStateBase* pipelineState) override;

	void destroy(Coral::SamplerBase* sampler) override;

	void destroy(Coral::SemaphoreBase* semaphore) override;

	void destroy(Coral::ShaderModuleBase* shaderModule) override;

	void destroy(Coral::SwapchainBase* surface) override;

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

	template<typename T, typename U, typename CreateError, typename ...InitArgs>
	std::expected<std::unique_ptr<T, Deleter<T>>, CreateError> create(InitArgs... args)
	{
		auto obj = new U(*this);

		if (auto error = obj->init(args...))
		{
			delete obj;
			return std::unexpected(*error);
		}

		return { std::unique_ptr<T, Deleter<T>>(obj) };
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