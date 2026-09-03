#ifndef CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
#define CORAL_VULKAN_COMMANDBUFFERIMPL_HPP

#include "CommandBuffer.hpp"

#include "Fwd.hpp"
#include "Resource.hpp"
#include "Vulkan.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace Coral::Vulkan
{

/*!
 * Implementation of the CommandBuffer interface using the Vulkan backend
 */
class CommandBufferImpl : public Coral::CommandBuffer,
                          public std::enable_shared_from_this<CommandBufferImpl>,
                          public Resource
{
public:

    CommandBufferImpl(CommandQueueImpl& commandQueue);

    virtual ~CommandBufferImpl();

    bool init(const CommandBuffer::CreateConfig& config);

    bool begin() override; 

    bool end() override;

    bool cmdBeginRenderPass(const BeginRenderPassInfo& info) override;

    bool cmdEndRenderPass() override;

    bool cmdClearImage(Coral::ImagePtr image, const CoClearColor& clearColor) override;

    bool cmdCopyBuffer(const CopyBufferInfo& info) override;

    bool cmdCopyImage(const CopyImageInfo& info) override;

    bool cmdBindVertexBuffer(Coral::BufferPtr buffer, uint32_t binding, size_t offset, size_t stride) override;

    bool cmdBindIndexBuffer(Coral::BufferPtr buffer, CoIndexFormat format, size_t offset) override;

    bool cmdBindPipeline(Coral::PipelineStatePtr pipeline) override;

    bool cmdBindDescriptorSet(Coral::DescriptorSetPtr descriptorSet, uint32_t index)  override;

    bool cmdDraw(const CoDrawInfo& info) override;
    
    bool cmdDrawIndirect(Coral::BufferPtr buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) override;

    bool cmdDrawIndexed(const CoDrawIndexedInfo& info) override;

    bool cmdDrawIndexedIndirect(Coral::BufferPtr buffer, uint64_t offset, uint32_t drawCount, uint32_t stride) override;

    bool cmdSetViewport(const CoViewportInfo& info) override;

    bool cmdUpdateBufferData(const Coral::UpdateBufferDataInfo& info) override;

    bool cmdUpdateImageData(const Coral::UpdateImageDataInfo& info) override;

    bool cmdGenerateMipMaps(Coral::ImagePtr image) override;

    bool cmdBlitImage(Coral::ImagePtr source, Coral::ImagePtr dest) override;

    VkCommandBuffer getVkCommandBuffer();

    [[nodiscard]] std::unordered_set<ResourcePtr> releaseRetainedResources();

private:

    void cmdBindCachedDescriptorSets();

    CommandQueueImpl& mCommandQueue;

    VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };

    VkCommandPool mCommandPool{ VK_NULL_HANDLE };

    std::string mName;

    bool mRetainReferences{ false };

    PipelineStateImplPtr mLastBoundPipelineState{ nullptr };

    std::unordered_map<uint32_t, DescriptorSetImplPtr> mCachedDescriptorSets;

    std::unordered_set<ResourcePtr> mRetainedResources;

}; // class CommandBufferImpl

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
