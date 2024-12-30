#include "ImageImpl.hpp"
#include "VulkanFormat.hpp"

#include <cmath>
#include <cstring>

using namespace Coral::Vulkan;

namespace
{

VkImageAspectFlags
getAspectFlags(Coral::PixelFormat format)
{
	switch (format)
	{
		case Coral::PixelFormat::DEPTH16:
		case Coral::PixelFormat::DEPTH32_F:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case Coral::PixelFormat::DEPTH24_STENCIL8:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
	}
}


VkImageUsageFlags
getUsageFlags(Coral::PixelFormat format)
{
	VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	if (Coral::Vulkan::isDepthFormat(format))
	{
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	return flags;
}


} // namespace
ImageImpl::~ImageImpl()
{
	if (!mContext)
	{
		return;
	}
	
	if (mImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(mContext->getVkDevice(), mImageView, nullptr);
	}

	if (mImage != VK_NULL_HANDLE && mIsOwner)
	{
		vmaDestroyImage(mContext->getVmaAllocator(), mImage, mAllocation);
	}
}


bool
ImageImpl::init(ContextImpl& context, VkImage image, Coral::PixelFormat format, uint32_t width, uint32_t height, uint32_t mipLevelCount, VkImageLayout layout)
{
	mContext		= &context;
	mImage			= image;
	mFormat			= format;
	mWidth			= width;
	mHeight			= height;
	mMipLevelCount	= mipLevelCount;
	mLayout			= layout;
	mIsOwner		= false;

	VkImageViewCreateInfo viewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewCreateInfo.image							= image;
	viewCreateInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format							= convert(format);
	viewCreateInfo.subresourceRange.aspectMask		= getAspectFlags(format);
	viewCreateInfo.subresourceRange.baseArrayLayer	= 0;
	viewCreateInfo.subresourceRange.layerCount		= 1;
	viewCreateInfo.subresourceRange.baseMipLevel	= 0;
	viewCreateInfo.subresourceRange.levelCount		= mMipLevelCount;

	if (vkCreateImageView(mContext->getVkDevice(), &viewCreateInfo, nullptr, &mImageView) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}


std::optional<Coral::ImageCreationError>
ImageImpl::init(ContextImpl& context, const Coral::ImageCreateConfig& config)
{
	mContext	= &context;
	mFormat		= config.format;
	mWidth		= config.width;
	mHeight		= config.height;
	mIsOwner	= true;

	if (config.hasMips)
	{
		mMipLevelCount = static_cast<uint32_t>(std::floor(std::log2(std::max(mWidth, mHeight)))) + 1;
	}
	else
	{
		mMipLevelCount = 1;
	}

	VkImageCreateInfo createInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	createInfo.imageType		= VK_IMAGE_TYPE_2D;
	createInfo.arrayLayers		= 1;
	createInfo.extent.width		= config.width;
	createInfo.extent.height	= config.height;
	createInfo.extent.depth		= 1;
	createInfo.mipLevels		= mMipLevelCount;
	createInfo.format			= convert(config.format);
	createInfo.tiling			= VK_IMAGE_TILING_OPTIMAL;
	createInfo.sharingMode		= VK_SHARING_MODE_EXCLUSIVE;
	createInfo.samples			= VK_SAMPLE_COUNT_1_BIT;
	createInfo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage			= getUsageFlags(config.format);

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VmaAllocationInfo info{};
	if (vmaCreateImage(mContext->getVmaAllocator(), &createInfo, &allocCreateInfo, &mImage, &mAllocation, &info) != VK_SUCCESS)
	{
		return ImageCreationError::INTERNAL_ERROR;
	}

	if (!init(context, mImage, config.format, config.width, config.height, mMipLevelCount, VK_IMAGE_LAYOUT_GENERAL))
	{
		return ImageCreationError::INTERNAL_ERROR;
	}

	mIsOwner = true;

	return {};
}


VkImage
ImageImpl::getVkImage()
{
	return mImage;
}


VkImageView
ImageImpl::getVkImageView()
{
	return mImageView;
}


VkImageLayout
ImageImpl::getVkImageLayout() const
{
	return mLayout;
}


uint32_t
ImageImpl::width() const
{
	return mWidth;
}


uint32_t
ImageImpl::height() const
{
	return mHeight;
}


Coral::PixelFormat
ImageImpl::format() const
{
	return mFormat;
}


uint32_t 
ImageImpl::getMipLevels() const
{
	return mMipLevelCount;
}


bool
ImageImpl::presentable() const
{
	return mLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}
