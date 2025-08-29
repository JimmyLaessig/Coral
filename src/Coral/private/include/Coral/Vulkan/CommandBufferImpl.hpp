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

	bool init(const CommandBufferCreateConfig& config);

	bool begin() override; 

	bool end() override;

	bool cmdBeginRenderPass(const BeginRenderPassInfo& info) override;

	bool cmdEndRenderPass() override;

	bool cmdClearImage(Coral::Image* image, const ClearColor& clearColor) override;

	bool cmdCopyBuffer(const CopyBufferInfo& info) override;

	bool cmdCopyImage(const CopyImageInfo& info) override;

	bool cmdBindVertexBuffer(Coral::Buffer* buffer, uint32_t binding, size_t offset, size_t stride) override;

	bool cmdBindIndexBuffer(Coral::Buffer* buffer, IndexFormat format, size_t offset) override;

	bool cmdBindPipeline(Coral::PipelineState* pipeline) override;

	bool cmdDrawIndexed(const DrawIndexInfo& info) override;

	bool cmdSetViewport(const Coral::ViewportInfo& info) override;

	bool cmdUpdateBufferData(const Coral::UpdateBufferDataInfo& info) override;

	bool cmdUpdateImageData(const Coral::UpdateImageDataInfo& info) override;

	void cmdBindDescriptor(Coral::Buffer* buffer, uint32_t binding) override;

	void cmdBindDescriptor(Coral::Image* image, Coral::Sampler* sampler, uint32_t binding) override;

	void cmdBindDescriptor(Coral::Sampler* sampler, uint32_t binding) override;

	void cmdBindDescriptor(Coral::Image* image, uint32_t binding) override;

	void cmdBlitImage(Coral::Image* source, Coral::Image* dest) override;

	VkCommandBuffer getVkCommandBuffer();

	[[nodiscard]] std::vector<std::shared_ptr<Coral::Buffer>> getStagingBuffers();

	void cmdAddImageBarrier(Coral::Vulkan::ImageImpl* image,
							uint32_t baseMipLevel,
							uint32_t levelCount,
							VkImageLayout oldLayout,
							VkImageLayout newLayout,
							VkAccessFlagBits srcAccessMask,
							VkAccessFlags dstAccessMask,
							VkPipelineStageFlags srcStageFlags,
							VkPipelineStageFlags dstStageFlags);

private:

	void cmdBindCachedDescriptors();

	CommandQueueImpl& mCommandQueue;

	VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };

	std::string mName;

	PipelineStateImpl* mLastBoundPipelineState{ nullptr };

	std::vector<std::shared_ptr<Coral::Buffer>> mStagingBuffers;

	std::unordered_map<uint32_t, std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>> mCachedDescriptorInfos;

	std::vector<VkWriteDescriptorSet> mDescriptorWrites;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
