#include "CommandBufferImpl.hpp"

#include <Coral/Types.hpp>

#include <cassert>
#include <mutex>
#include <optional>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

std::optional<VkIndexType>
convert(Coral::AttributeFormat format)
{
	switch (format)
	{
		case Coral::AttributeFormat::UINT32:
			return VK_INDEX_TYPE_UINT32;
		case Coral::AttributeFormat::UINT16:
			return VK_INDEX_TYPE_UINT16;
		default:
			return {};
	}
}


VkAttachmentLoadOp
convert(Coral::ClearOp clearOp)
{
	switch (clearOp)
	{
	case Coral::ClearOp::DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	case Coral::ClearOp::LOAD:		return VK_ATTACHMENT_LOAD_OP_LOAD;
	case Coral::ClearOp::CLEAR:		return VK_ATTACHMENT_LOAD_OP_CLEAR;
	default:
		assert(false);
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}


} // namespace

void
CommandBufferImpl::cmdAddImageBarrier(Coral::Vulkan::ImageImpl* image,
								  uint32_t baseMipLevel, 
								  uint32_t levelCount,
								  VkImageLayout oldLayout, 
								  VkImageLayout newLayout, 
								  VkAccessFlagBits srcAccessMask, 
								  VkAccessFlags dstAccessMask,
								  VkPipelineStageFlags srcStageFlags,
								  VkPipelineStageFlags dstStageFlags)
{
	VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout						= oldLayout;
	barrier.newLayout						= newLayout;
	barrier.srcQueueFamilyIndex				= mContext->getQueueFamilyIndex();
	barrier.dstQueueFamilyIndex				= mContext->getQueueFamilyIndex();
	barrier.image							= image->getVkImage();
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= baseMipLevel;
	barrier.subresourceRange.levelCount		= levelCount;
	barrier.subresourceRange.layerCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.srcAccessMask					= srcAccessMask;
	barrier.dstAccessMask					= dstAccessMask;

	vkCmdPipelineBarrier(mCommandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}


CommandBufferImpl::~CommandBufferImpl()
{
	if (mCommandPool && mCommandBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(mContext->getVkDevice(), mCommandPool, 1, &mCommandBuffer);
	}
}


bool
CommandBufferImpl::init(Coral::Vulkan::CommandQueueImpl& queue, const Coral::CommandBufferCreateConfig& config)
{
	mContext     = &queue.context();
	mName	     = config.name;
	mCommandPool = queue.getVkCommandPool();
	auto device	 = mContext->getVkDevice();

	VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool			= mCommandPool;
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount	= 1;
	if (vkAllocateCommandBuffers(device, &allocInfo, &mCommandBuffer) != VK_SUCCESS)
	{
		return false;
	}
	
	return true;
}


bool
CommandBufferImpl::begin()
{
	VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	
	return vkBeginCommandBuffer(mCommandBuffer, &info) == VK_SUCCESS;
}


bool
CommandBufferImpl::end()
{
	return vkEndCommandBuffer(mCommandBuffer) == VK_SUCCESS;
}


bool
CommandBufferImpl::cmdBeginRenderPass(const Coral::BeginRenderPassInfo& info)
{
	if (info.framebuffer == nullptr)
	{
		return false;
	}
	auto framebuffer = static_cast<Coral::Vulkan::FramebufferImpl*>(info.framebuffer);

	const auto& colorAttachments = framebuffer->colorAttachments();
	const auto& depthAttachment = framebuffer->depthAttachment();

	// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before rendering, we
	// must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL). Once rendering is
	// done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR'in order to present the swapchain
	// image. Both layout transitions must be executed manually via memory barriers. During cmdBeginRenderPass we
	// recorded all swapchain images and after ending the rendering we insert memory barriers to ensure that the images
	// have a presentable layout.
	std::vector<VkImageMemoryBarrier> imageBarriers;
	for (const auto& colorAttachment : colorAttachments)
	{
		auto image = static_cast<Coral::Vulkan::ImageImpl*>(colorAttachment.image);
		if (image->presentable())
		{
			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.dstAccessMask					= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.oldLayout						= VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.image							= image->getVkImage();
			barrier.srcQueueFamilyIndex				= mContext->getQueueFamilyIndex();
			barrier.dstQueueFamilyIndex				= mContext->getQueueFamilyIndex();
			barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount		= 1;
			barrier.subresourceRange.baseMipLevel	= 0;
			barrier.subresourceRange.levelCount		= image->getMipLevels();

			imageBarriers.push_back(barrier);

			mPresentableImagesInUse.push_back(image);
		}
	}

	if (!imageBarriers.empty())
	{
		vkCmdPipelineBarrier(mCommandBuffer,
							 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,				// srcStageMask
							 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
							 0,
							 0,
							 nullptr,
							 0,
							 nullptr,
							 static_cast<uint32_t>(imageBarriers.size()),
							 imageBarriers.data());
	}


	uint32_t i{ 0 };
	std::vector<VkRenderingAttachmentInfo> attachments;
	for (const auto& colorAttachment : colorAttachments)
	{
		auto image = static_cast<Coral::Vulkan::ImageImpl*>(colorAttachment.image);

		VkRenderingAttachmentInfo attachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
		attachmentInfo.loadOp			= convert(colorAttachment.clearOp);
		attachmentInfo.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		attachmentInfo.imageLayout		= VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;//  image->getVkImageLayout();
		attachmentInfo.imageView		= image->getVkImageView();
		attachmentInfo.resolveMode		= VK_RESOLVE_MODE_NONE;
		attachmentInfo.resolveImageView = VK_NULL_HANDLE;
		
		if (colorAttachments.size() == info.clearColor.size())
		{
			attachmentInfo.clearValue.color.float32[0] = info.clearColor[i].color[0];
			attachmentInfo.clearValue.color.float32[1] = info.clearColor[i].color[1];
			attachmentInfo.clearValue.color.float32[2] = info.clearColor[i].color[2];
			attachmentInfo.clearValue.color.float32[3] = info.clearColor[i].color[3];
		}
		
		attachments.push_back(attachmentInfo);
		i++;
	}

	VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
	renderingInfo.colorAttachmentCount		= static_cast<uint32_t>(attachments.size());
	renderingInfo.pColorAttachments			= attachments.data();
	renderingInfo.layerCount				= 1;
	renderingInfo.renderArea.offset.x		= 0;
	renderingInfo.renderArea.offset.y		= 0;
	renderingInfo.renderArea.extent.width	= framebuffer->width();
	renderingInfo.renderArea.extent.height	= framebuffer->height();

	VkRenderingAttachmentInfo depthAttachmentInfo{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };

	if (depthAttachment)
	{
		auto image = static_cast<Coral::Vulkan::ImageImpl*>(depthAttachment->image);
		
		depthAttachmentInfo.loadOp				= convert(depthAttachment->clearOp);
		depthAttachmentInfo.storeOp				= VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachmentInfo.imageLayout			= VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;// image->getVkImageLayout();
		depthAttachmentInfo.imageView			= image->getVkImageView();
		depthAttachmentInfo.resolveMode			= VK_RESOLVE_MODE_NONE;
		depthAttachmentInfo.resolveImageView	= VK_NULL_HANDLE;

		if (info.clearDepth)
		{
			depthAttachmentInfo.clearValue.depthStencil.depth	= info.clearDepth->depth;
			depthAttachmentInfo.clearValue.depthStencil.stencil = info.clearDepth->stencil;
		}

		renderingInfo.pDepthAttachment		= &depthAttachmentInfo;
		renderingInfo.pStencilAttachment	= &depthAttachmentInfo;
	}


	vkCmdBeginRendering(mCommandBuffer, &renderingInfo);

	return true;
}


bool
CommandBufferImpl::cmdEndRenderPass()
{
	vkCmdEndRendering(mCommandBuffer);

	if (!mPresentableImagesInUse.empty())
	{
		// Dynamic rendering requires manual image layout transition for presentable swapchain images. Before
		// rendering, we must transition the image to a supported layout (VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL).
		// Once rendering is done, the image layout must transition to 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR' in order to
		// present the swapchain image. Both layout transitions must be executed manually via memory barriers. During
		// cmdBeginRenderPass we recorded all swapchain images and after ending the rendering we insert memory barriers
		// to ensure that the images have a presentable layout.

		std::vector<VkImageMemoryBarrier> imageBarriers;
		for (auto image : mPresentableImagesInUse)
		{
			VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.srcAccessMask					= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.oldLayout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout						= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.image							= image->getVkImage();
			barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount		= 1;
			barrier.subresourceRange.baseMipLevel	= 0;
			barrier.subresourceRange.levelCount		= image->getMipLevels();

			imageBarriers.push_back(barrier);
		}
		
		vkCmdPipelineBarrier(mCommandBuffer,
							 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // srcStageMask
							 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,			// dstStageMask
							 0,
							 0,
							 nullptr,
							 0,
							 nullptr,
							 static_cast<uint32_t>(mPresentableImagesInUse.size()), // imageMemoryBarrierCount
							 imageBarriers.data());

		mPresentableImagesInUse.clear();
	}

	return true;
}


bool 
CommandBufferImpl::cmdClearImage(Coral::Image* image, const Coral::ClearColor& clearColor)
{
	if (image->presentable())
	{
		return false;
	}

	auto vkImage = static_cast<Coral::Vulkan::ImageImpl*>(image);
	
	VkClearColorValue color;
	color.float32[0] = clearColor.color[0];
	color.float32[1] = clearColor.color[1];
	color.float32[2] = clearColor.color[2];
	color.float32[3] = clearColor.color[3];

	std::vector<VkImageSubresourceRange> ranges;

	for (uint32_t i = 0; i < image->getMipLevels(); ++i)
	{
		auto& range				= ranges.emplace_back();
		range.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseArrayLayer	= 0;
		range.layerCount		= 1;
		range.baseMipLevel		= i;
		range.levelCount		= image->getMipLevels();
	}

	vkCmdClearColorImage(mCommandBuffer, 
						 vkImage->getVkImage(), 
						 VK_IMAGE_LAYOUT_GENERAL,
						 &color, 
						 static_cast<uint32_t>(ranges.size()), 
						 ranges.data());

	return true;
}


bool
CommandBufferImpl::cmdCopyBuffer(const CopyBufferInfo& info)
{
	auto source = static_cast<Coral::Vulkan::BufferImpl*>(info.source);
	auto dest = static_cast<Coral::Vulkan::BufferImpl*>(info.dest);

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset	= info.sourceOffset;
	bufferCopy.dstOffset	= info.destOffset;
	bufferCopy.size			= info.size;
	vkCmdCopyBuffer(mCommandBuffer, source->getVkBuffer(), dest->getVkBuffer(), 1, &bufferCopy);

	return true;
}


bool
CommandBufferImpl::cmdCopyImage(const CopyImageInfo& info)
{
	//auto buffer = static_cast<CO
	//vkCmdCopyBufferToImage(mCommandBuffer, info.source)
	return false;
}


bool
CommandBufferImpl::cmdBindVertexBuffer(Coral::BufferView* vertexBuffer, uint32_t binding)
{
	auto bufferView = static_cast<Coral::Vulkan::BufferViewImpl*>(vertexBuffer);
	auto buffer		= static_cast<Coral::Vulkan::BufferImpl*>(bufferView->buffer());

	if (buffer->type() != BufferType::VERTEX_BUFFER)
	{
		return false;
	}
	auto vkBuffer       = buffer->getVkBuffer();
	VkDeviceSize offset = bufferView->offset();
	VkDeviceSize size	= bufferView->count() * bufferView->stride() - offset;
	VkDeviceSize stride = bufferView->stride();

	vkCmdBindVertexBuffers2(mCommandBuffer, binding, 1, &vkBuffer, &offset, &size, &stride);

	return true;
}


bool
CommandBufferImpl::cmdBindIndexBuffer(Coral::BufferView* indexBuffer)
{
	auto bufferView = static_cast<Coral::Vulkan::BufferViewImpl*>(indexBuffer);
	auto buffer = static_cast<Coral::Vulkan::BufferImpl*>(bufferView->buffer());
		
	if (buffer->type() != BufferType::INDEX_BUFFER)
	{
		// TODO bufer type must be index buffer
		return false;
	}

	VkIndexType indexType{};
	switch (bufferView->attributeFormat())
	{
		case AttributeFormat::UINT16 :
			indexType = VK_INDEX_TYPE_UINT16;
			break;
		case AttributeFormat::UINT32:
			indexType = VK_INDEX_TYPE_UINT32;
			break;
		default:
			//TODO Invalid format
			return false;
	}

	if (bufferView->stride() != sizeInBytes(bufferView->attributeFormat()))
	{
		// TODO: INDEX BUFFER MUST BE TIGHTLY PACKED
		return false;
	}

	vkCmdBindIndexBuffer(mCommandBuffer, buffer->getVkBuffer(), bufferView->offset(), indexType);

	return true;
}


bool
CommandBufferImpl::cmdBindPipeline(Coral::PipelineState* pipelineState)
{
	mLastBoundPipelineState = static_cast<Coral::Vulkan::PipelineStateImpl*>(pipelineState);
	vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mLastBoundPipelineState->getVkPipeline());

	return true;
}


bool
CommandBufferImpl::cmdDrawIndexed(const DrawIndexInfo& info)
{
	vkCmdDrawIndexed(mCommandBuffer, info.indexCount, 1, info.firstIndex, 0, 0);

	return true;
}


bool
CommandBufferImpl::cmdSetViewport(const Coral::ViewportInfo& info)
{
	if (info.maxDepth < info.minDepth)
	{
		return false;
	}

	if (info.viewport.width == 0 || info.viewport.height == 0)
	{
		return false;
	}

	VkViewport viewport;
	viewport.x			= static_cast<float>(info.viewport.x);
	viewport.y			= static_cast<float>(info.viewport.y);
	viewport.width		= static_cast<float>(info.viewport.width);
	viewport.height		= static_cast<float>(info.viewport.height);
	viewport.minDepth	= static_cast<float>(info.minDepth);
	viewport.maxDepth	= static_cast<float>(info.maxDepth);

	// Vulkan and OpenGL use different coordinate systems. Per default, the Vulkan coordinate system is y-down. To
	// combat the inverted image, we need to flip the viewport by passing negative height and adjust the y-offset.
	if (info.mode == Coral::ViewportMode::Y_UP)
	{
		//viewport.x		= info.viewport.x;
		viewport.y		= viewport.height + viewport.y;
		viewport.width	= viewport.width;
		viewport.height = -viewport.height;
	}

	VkRect2D rect;
	rect.extent.width	= info.viewport.width;
	rect.extent.height	= info.viewport.height;
	rect.offset.x		= info.viewport.x;
	rect.offset.y		= info.viewport.y;

	vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(mCommandBuffer, 0, 1, &rect);

	return true;
}


bool
CommandBufferImpl::cmdBindDescriptorSet(Coral::DescriptorSet* descriptorSet, uint32_t index)
{
	if (!mLastBoundPipelineState)
	{
		// TODO LOG ERROR
		return false;
	}

	auto layout = mLastBoundPipelineState->getVkPipelineLayout();

	auto impl				= static_cast<Coral::Vulkan::DescriptorSetImpl*>(descriptorSet);
	auto vkDescriptorSet	= impl->getVkDescriptorSet();
	vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, index, 1, &vkDescriptorSet, 0, nullptr);

	return true;
}


bool
CommandBufferImpl::cmdUpdateBufferData(const Coral::UpdateBufferDataInfo& info)
{
	if (info.offset + info.data.size() > info.buffer->size())
	{
		return false;
	}

	auto buffer = static_cast<Coral::Vulkan::BufferImpl*>(info.buffer);

	auto stagingBuffer = mContext->requestStagingBuffer(info.data.size());
	auto stagingBufferVK = static_cast<Coral::Vulkan::BufferImpl*>(stagingBuffer.get());

	auto mapped = stagingBuffer->map();
	std::memcpy(mapped + info.offset, info.data.data(), info.data.size());
	stagingBuffer->unmap();

	VkBufferCopy bufferCopy;
	bufferCopy.srcOffset	= 0;
	bufferCopy.dstOffset	= info.offset;
	bufferCopy.size			= info.data.size();

	vkCmdCopyBuffer(mCommandBuffer, stagingBufferVK->getVkBuffer(), buffer->getVkBuffer(), 1, &bufferCopy);

	/*VkBufferMemoryBarrier barrier{};
	barrier.buffer = bufferImpl->getVkBuffer();
	barrier.dstQueueFamilyIndex = mContext->getQueueFamilyIndex();
	barrier.srcQueueFamilyIndex = mContext->getQueueFamilyIndex();*/

	/*VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.image = image->getVkImage();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = image->getMipLevels();*/

	//imageBarriers.push_back(barrier);

	//vkCmdPipelineBarrier(mCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, )
	// Store the temporary staging buffer until the command buffer was executed
	mStagingBuffers.push_back(std::move(stagingBuffer));

	return true;
}


bool
CommandBufferImpl::cmdUpdateImageData(const Coral::UpdateImageDataInfo& info)
{
	auto image = static_cast<Coral::Vulkan::ImageImpl*>(info.image);

	auto stagingBuffer = mContext->requestStagingBuffer(info.data.size());
	auto stagingBufferVK = static_cast<Coral::Vulkan::BufferImpl*>(stagingBuffer.get());

	auto mapped = stagingBuffer->map();
	std::memcpy(mapped, info.data.data(), info.data.size());
	stagingBuffer->unmap();

	// Before copying data, transition the image layout to be optimal for receiving data. 

	cmdAddImageBarrier(image, 
					   0,
					   image->getMipLevels(), 
					   VK_IMAGE_LAYOUT_UNDEFINED,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					   VK_ACCESS_TRANSFER_READ_BIT,
					   VK_ACCESS_TRANSFER_WRITE_BIT,
					   VK_PIPELINE_STAGE_TRANSFER_BIT,
					   VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy copy{};
	copy.bufferOffset		= 0;
	copy.bufferRowLength	= 0; // TODO: Support non-tightly packed images
	copy.bufferImageHeight	= 0; // TODO: Support non-tightly packed images
	copy.imageExtent.width	= image->width();
	copy.imageExtent.height	= image->height();
	copy.imageExtent.depth	= 1;
	copy.imageOffset.x		= 0;
	copy.imageOffset.y		= 0;
	copy.imageOffset.z		= 0;
	// Copy the data to the first mip level
	copy.imageSubresource.mipLevel			= 0;
	copy.imageSubresource.layerCount		= 1;
	copy.imageSubresource.baseArrayLayer	= 0;
	copy.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;

	vkCmdCopyBufferToImage(mCommandBuffer, stagingBufferVK->getVkBuffer(), image->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

	mStagingBuffers.push_back(std::move(stagingBuffer));

	// After copying the buffer data to the image, we need to transition the layout to be readable from a shader. This
	// transition is only needed if we don't generate mip maps

	if (info.updateMips && image->getMipLevels() > 1)
	{
		for (uint32_t i = 1; i < image->getMipLevels(); ++i)
		{
			// Transition the source image layout to TRANSFER SOURCE 
			cmdAddImageBarrier(image,
							   i - 1,
							   1,
							   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							   VK_ACCESS_TRANSFER_WRITE_BIT,
							   VK_ACCESS_SHADER_READ_BIT,
							   VK_PIPELINE_STAGE_TRANSFER_BIT,
							   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

			// Blit the src image into the dst image
			VkImageBlit imageBlit{};
			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel	= i - 1;
			imageBlit.srcOffsets[1].x			= int32_t(image->width() >> (i - 1));
			imageBlit.srcOffsets[1].y			= int32_t(image->height() >> (i - 1));
			imageBlit.srcOffsets[1].z			= 1;

			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel	= i;
			imageBlit.dstOffsets[1].x			= int32_t(image->width() >> i);
			imageBlit.dstOffsets[1].y			= int32_t(image->height() >> i);
			imageBlit.dstOffsets[1].z			= 1;

			vkCmdBlitImage(mCommandBuffer, image->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
					       image->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
		}
	}

	// Finally, transition all mip levels to optimal shader

	cmdAddImageBarrier(image,
					   0,
					   image->getMipLevels(),
					   VK_IMAGE_LAYOUT_UNDEFINED,
					   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					   VK_ACCESS_TRANSFER_WRITE_BIT,
					   VK_ACCESS_SHADER_READ_BIT,
					   VK_PIPELINE_STAGE_TRANSFER_BIT,
					   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	return true;
}


VkCommandBuffer 
CommandBufferImpl::getVkCommandBuffer()
{
	return mCommandBuffer;
}


std::vector<Coral::BufferPtr>
CommandBufferImpl::getStagingBuffers()
{
	std::vector<Coral::BufferPtr> stagingBuffers;
	stagingBuffers.swap(mStagingBuffers);

	return stagingBuffers;
}
