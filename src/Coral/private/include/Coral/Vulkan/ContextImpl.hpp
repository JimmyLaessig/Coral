#ifndef CORAL_VULKAN_CONTEXTIMPL_HPP
#define CORAL_VULKAN_CONTEXTIMPL_HPP

#include <Coral/Context.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <span>

namespace Coral
{
class BufferPool;
} // namespace Coral

namespace Coral::Vulkan
{

class ContextImpl : public Coral::Context
{
public:

    static std::shared_ptr<ContextImpl> create(const Context::CreateConfig& config);

    bool init(const Context::CreateConfig& config);

    virtual ~ContextImpl();

    CoGraphicsAPI graphicsAPI() const override { return CO_GRAPHICS_API_VULKAN; }

    Coral::CommandQueue* getGraphicsQueue() override;

    Coral::CommandQueue* getComputeQueue() override;
                                    
    Coral::CommandQueue* getTransferQueue() override;

    std::expected<Coral::BufferPtr, Coral::Buffer::CreateError> createBuffer(const Coral::Buffer::CreateConfig& config) override;

    std::expected<Coral::FencePtr, Coral::Fence::CreateError> createFence(const Coral::Fence::CreateConfig& config)  override;

    std::expected<Coral::FramebufferPtr, Coral::Framebuffer::CreateError> createFramebuffer(const Coral::Framebuffer::CreateConfig& config) override;

    std::expected<Coral::ImagePtr, Coral::Image::CreateError> createImage(const Coral::Image::CreateConfig& config) override;

    std::expected<Coral::PipelineStatePtr, Coral::PipelineState::CreateError> createPipelineState(const Coral::PipelineState::CreateConfig& config) override;

    std::expected<Coral::SamplerPtr, Coral::Sampler::CreateError> createSampler(const Coral::Sampler::CreateConfig& config) override;

    std::expected<Coral::SemaphorePtr, Coral::Semaphore::CreateError> createSemaphore(const Coral::Semaphore::CreateConfig& config) override;

    std::expected<Coral::ShaderModulePtr, Coral::ShaderModule::CreateError> createShaderModule(const Coral::ShaderModule::CreateConfig& config) override;

    std::expected<Coral::SwapchainPtr, Coral::Swapchain::CreateError> createSwapchain(const Coral::Swapchain::CreateConfig& config) override;

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
    BufferImplPtr requestStagingBuffer(size_t bufferSize);

private:
    
    template<typename T, typename U, typename CreateError, typename ...InitArgs>
    std::expected<std::shared_ptr<T>,  CreateError> create(InitArgs... args)
    {
        auto obj = std::make_shared<U>(*this);
        if (auto error = obj->init(args...))
        {
            return std::unexpected(*error);
        }

        return obj;
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