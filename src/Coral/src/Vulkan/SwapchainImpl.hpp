#ifndef CORAL_VULKAN_SWAPCHAINIMPL_HPP
#define CORAL_VULKAN_SWAPCHAINIMPL_HPP

#include "Swapchain.hpp"

#include "Fwd.hpp"
#include "Resource.hpp"
#include "Vulkan.hpp"

#include <memory>

#include <mutex>

namespace Coral::Vulkan
{

/*!
 * Implementation of the Swapchain interface using the Vulkan backend
 */
class SwapchainImpl : public Coral::Swapchain
                    , public Resource
                    , public std::enable_shared_from_this<SwapchainImpl>
{
public:

    using Resource::Resource;

    virtual ~SwapchainImpl();

    std::optional<Coral::Swapchain::CreateError> init(const Coral::Swapchain::CreateConfig& config);

    void* nativeWindowHandle() override;

    AcquiredImageInfo acquireNextSwapchainImage(FencePtr fence) override;

    uint32_t currentSwapchainImageIndex() const override;

    uint32_t swapchainImageCount() const override;

    CoExtent swapchainExtent() const override;

    VkSurfaceKHR getVkSurface();

    VkSwapchainKHR getVkSwapchain();

    const Coral::Swapchain::CreateConfig& config() const { return mConfig; };

    void present(CommandQueueImpl& commandQueue, const std::vector<SemaphorePtr>& waitSemaphores);

private:

    bool initSwapchain(const Coral::Swapchain::CreateConfig& config);

    void* mNativeWindowHandle{ nullptr };

    VkSurfaceKHR mSurface{ VK_NULL_HANDLE };

    VkSwapchainKHR mSwapchain{ VK_NULL_HANDLE };

    VkSurfaceFormatKHR mSurfaceFormat{};

    uint32_t mSwapchainImageCount{ 0 };

    uint32_t mCurrentSwapchainIndex{ 0 };

    VkExtent2D mSwapchainExtent{ 0, 0 };

    // Semaphores to be signaled once the swapchain image has been acquired (one for each swapchain image).
    std::vector<Coral::SemaphorePtr> mImageAcquiredSemaphore;
    
    // Commandbuffers to perform layout transition to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL after acquisition
    // (one for each swapchain image).
    std::vector<Coral::CommandBufferPtr> mTransitionToColorAttachment;

    // Semaphores to be signaled once the swapchain image has been acquired and it's memory layout is transitioned
    // to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL (one for each swapchain image).
    std::vector<Coral::SemaphorePtr> mImageReadySemaphore;

    // Commandbuffers to perform layout transition to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR (one for each swapchain image).
    std::vector<Coral::CommandBufferPtr> mTransitionToPresent;

    // Semaphores to be signaled once the swapchain image memory layout has transitioned to
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR (one for each swapchain image).
    std::vector<Coral::SemaphorePtr> mImagePresentableSemaphore;

    // Swapchain framebuffers  (one for each swapchain image).
    std::vector<Coral::FramebufferPtr> mFramebuffers;

    // The swapchain image
    std::vector<Coral::ImagePtr> mSwapchainImages;

    Coral::ImagePtr mSwapchainDepthImage;

    Coral::Swapchain::CreateConfig mConfig{};

    mutable std::mutex mThreadProtection;

}; // class SwapchainImpl

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_SWAPCHAINIMPL_HPP
