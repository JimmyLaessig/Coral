#include <Coral/Vulkan/FramebufferImpl.hpp>

#include <Coral/Vulkan/VulkanFormat.hpp>

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <vector>

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
	if (!config.colorAttachments.empty())
	{
		return VkExtent2D{ config.colorAttachments.front().image->width(), config.colorAttachments.front().image->height() };
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
	auto extent = getFramebufferExtent(config);

	if (!extent)
	{
		return FramebufferCreationError::INTERNAL_ERROR;
	}

	mWidth  = extent->width;
	mHeight = extent->height;

	mColorAttachments.assign_range(config.colorAttachments);
	mDepthAttachment = config.depthAttachment;

	// Sort the color attachments by attachment index for easier validation
	std::ranges::sort(mColorAttachments, {}, &Coral::ColorAttachment::attachment);

	// Validate the attachments:
	// 1. The image format must not be a depth format
	// 2. The attachment index must be unique
	// 3. The depth attachment format must be a depth format

	bool colorFormatsValid = std::ranges::none_of(config.colorAttachments, [](const auto& attachment)
	{
		return isDepthFormat(attachment.image->format());
	});

	if (!colorFormatsValid)
	{
		return FramebufferCreationError::INVALID_COLOR_ATTACHMENT_FORMAT;
	}

	// Check if the color attachments are unique
	bool attachmentsValid = std::ranges::adjacent_find(mColorAttachments, {}, &Coral::ColorAttachment::attachment) == mColorAttachments.end();
	if (!attachmentsValid)
	{
		return FramebufferCreationError::DUPLICATE_COLOR_ATTACHMENTS;
	}

	if (mDepthAttachment)
	{
		if (!isDepthFormat(mDepthAttachment->image->format()))
		{
			return FramebufferCreationError::INTERNAL_ERROR;
		}
	}

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
FramebufferImpl::signature()
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


Coral::Image*
FramebufferImpl::colorAttachment(uint32_t attachment)
{
	for (const auto& ca : mColorAttachments)
	{
		if (ca.attachment == attachment)
		{
			return ca.image;
		}
	}
	return nullptr;
}
