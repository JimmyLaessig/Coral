#include <Coral/Vulkan/FramebufferImpl.hpp>

#include <Coral/Vulkan/VulkanFormat.hpp>

#include <vector>
#include <cassert>
#include <optional>

using namespace Coral::Vulkan;

namespace
{

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


std::optional<VkExtent2D>
getFramebufferExtent(const Coral::FramebufferCreateConfig& config)
{
	if (!config.colorAttachment.empty())
	{
		return VkExtent2D{ config.colorAttachment.front().image->width(), config.colorAttachment.front().image->height() };
	}
	else if (config.depthAttachment)
	{
		return VkExtent2D{ config.depthAttachment->image->width(), config.depthAttachment->image->height() };
	}

	return {};
}

} // namespace


std::optional<Coral::FramebufferCreationError>
FramebufferImpl::init(const Coral::FramebufferCreateConfig& config)
{
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	attachmentDescriptions.reserve(config.colorAttachment.size());

	std::vector<VkAttachmentReference> attachmentReferences;
	attachmentReferences.reserve(config.colorAttachment.size());

	VkAttachmentReference depthStencilAttachmentRefernce;

	auto extent = getFramebufferExtent(config);

	if (!extent)
	{
		return FramebufferCreationError::INTERNAL_ERROR;
	}

	mWidth = extent->width;
	mHeight = extent->height;
	
	for (auto attachment : config.colorAttachment)
	{
		if (isDepthFormat(attachment.image->format()))
		{
			return FramebufferCreationError::INTERNAL_ERROR;
		}
		mColorAttachments.push_back(attachment);
	}
	
	if (config.depthAttachment)
	{
		if (!isDepthFormat(config.depthAttachment->image->format()))
		{
			return FramebufferCreationError::INTERNAL_ERROR;
		}
	}

	mDepthAttachment = config.depthAttachment;

	return {};
}


const std::vector<Coral::ColorAttachment>&
FramebufferImpl::colorAttachments()
{
	return mColorAttachments;
}

const std::optional<Coral::DepthAttachment>&
FramebufferImpl::depthAttachment()
{
	return mDepthAttachment;
}


uint32_t
FramebufferImpl::width() const
{
	return mWidth;
}


uint32_t
FramebufferImpl::height() const
{
	return mHeight;
}


Coral::FramebufferSignature
FramebufferImpl::getSignature()
{
	Coral::FramebufferSignature signature{};

	if (mDepthAttachment)
	{
		signature.depthStencilAttachmentFormat = mDepthAttachment->image->format();
	}

	for (const auto& colorAttachment : mColorAttachments)
	{
		signature.colorAttachmentFormats.push_back(colorAttachment.image->format());
	}

	return signature;
}