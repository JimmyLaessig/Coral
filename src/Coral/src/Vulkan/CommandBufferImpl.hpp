#ifndef CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
#define CORAL_VULKAN_COMMANDBUFFERIMPL_HPP

#include <Coral/CommandBuffer.hpp>
#include <Coral/RAII.hpp>

#include "BufferImpl.hpp"
#include "BufferViewImpl.hpp"
#include "CommandQueueImpl.hpp"
#include "FramebufferImpl.hpp"
#include "ImageImpl.hpp"
#include "PipelineStateImpl.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Coral::Vulkan
{

class CommandBufferImpl : public Coral::CommandBuffer
{
public:

	virtual ~CommandBufferImpl();

	bool init(Coral::Vulkan::CommandQueueImpl& queue, const CommandBufferCreateConfig& config);

	bool begin() override; 

	bool end() override;

	bool cmdBeginRenderPass(const BeginRenderPassInfo& info) override;

	bool cmdEndRenderPass() override;

	bool cmdClearImage(Coral::Image* image, const ClearColor& clearColor) override;

	bool cmdCopyBuffer(const CopyBufferInfo& info) override;

	bool cmdCopyImage(const CopyImageInfo& info) override;

	bool cmdBindVertexBuffer(Coral::BufferView* bufferView, uint32_t binding) override;

	bool cmdBindIndexBuffer(Coral::BufferView* bufferView) override;

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

	void cmdBindCachedDescriptorSet();

	

	std::vector<Coral::Vulkan::ImageImpl*> mPresentableImagesInUse;

	ContextImpl* mContext{ nullptr };

	VkCommandPool mCommandPool{ VK_NULL_HANDLE };

	VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };

	std::string mName;

	PipelineStateImpl* mLastBoundPipelineState{ nullptr };

	std::vector<Coral::DescriptorSet*> mUnboundDescriptorSets;

	std::vector<std::shared_ptr<Coral::Buffer>> mStagingBuffers;

	std::unordered_map<uint32_t, std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>> mCachedDescriptorInfos;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_COMMANDBUFFERIMPL_HPP
