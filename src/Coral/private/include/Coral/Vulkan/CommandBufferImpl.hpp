#ifndef CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
#define CORAL_VULKAN_COMMANDBUFFERIMPL_HPP

#include <Coral/CommandBuffer.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Coral::Vulkan
{

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

    bool cmdDrawIndexed(const DrawIndexInfo& info) override;

    bool cmdSetViewport(const CoViewportInfo& info) override;

    bool cmdUpdateBufferData(const Coral::UpdateBufferDataInfo& info) override;

    bool cmdUpdateImageData(const Coral::UpdateImageDataInfo& info) override;

    bool cmdGenerateMipMaps(Coral::ImagePtr image) override;

    void cmdBindDescriptor(Coral::BufferPtr buffer, uint32_t binding) override;

    void cmdBindDescriptor(Coral::SamplerPtr sampler, uint32_t binding) override;

    void cmdBindDescriptor(Coral::ImagePtr image, uint32_t binding) override;

    bool cmdBlitImage(Coral::ImagePtr source, Coral::ImagePtr dest) override;

    VkCommandBuffer getVkCommandBuffer();

    [[nodiscard]] std::unordered_set<ResourcePtr> releaseRetainedResources();

private:

    void cmdBindCachedDescriptors();

    CommandQueueImpl& mCommandQueue;

    VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };

    VkCommandPool mCommandPool{ VK_NULL_HANDLE };

    std::string mName;

    bool mRetainReferences{ false };

    PipelineStateImplPtr mLastBoundPipelineState{ nullptr };

    std::unordered_set<ResourcePtr> mRetainedResources;

    std::unordered_map<uint32_t, std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>> mCachedDescriptorInfos;

    std::vector<VkWriteDescriptorSet> mDescriptorWrites;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
