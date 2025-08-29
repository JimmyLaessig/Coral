#include <Coral/Vulkan/ImageImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>

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
	if (mImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(context().getVkDevice(), mImageView, nullptr);
	}

	if (mImage != VK_NULL_HANDLE && mIsOwner)
	{
		vmaDestroyImage(context().getVmaAllocator(), mImage, mAllocation);
	}
}


bool
ImageImpl::init(VkImage image, Coral::PixelFormat format, uint32_t width, uint32_t height, uint32_t mipLevelCount, ImageUsageHint usageHint)
{
	mImage			= image;
	mFormat			= format;
	mWidth			= width;
	mHeight			= height;
	mMipLevelCount	= mipLevelCount;
	mIsOwner		= false;

	mCurrentLayout.resize(mMipLevelCount, VK_IMAGE_LAYOUT_UNDEFINED);

	switch (usageHint)
	{
		case ImageUsageHint::FRAMEBUFFER_ATTACHMENT:
			mPreferredImageLayout = isDepthFormat(format) ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case ImageUsageHint::SHADER_READ_ONLY:
			mPreferredImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		default:
			std::unreachable();
	}

	VkImageViewCreateInfo viewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewCreateInfo.image							= image;
	viewCreateInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format							= convert(format);
	viewCreateInfo.subresourceRange.aspectMask		= getAspectFlags(format);
	viewCreateInfo.subresourceRange.baseArrayLayer	= 0;
	viewCreateInfo.subresourceRange.layerCount		= 1;
	viewCreateInfo.subresourceRange.baseMipLevel	= 0;
	viewCreateInfo.subresourceRange.levelCount		= mMipLevelCount;

	if (vkCreateImageView(context().getVkDevice(), &viewCreateInfo, nullptr, &mImageView) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}


std::optional<Coral::ImageCreationError>
ImageImpl::init(const Coral::ImageCreateConfig& config)
{
	mFormat		= config.format;
	mWidth		= config.width;
	mHeight		= config.height;
	mIsOwner	= true;

	if (config.hasMipMaps)
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
	if (vmaCreateImage(context().getVmaAllocator(), &createInfo, &allocCreateInfo, &mImage, &mAllocation, &info) != VK_SUCCESS)
	{
		return ImageCreationError::INTERNAL_ERROR;
	}

	if (!init(mImage, config.format, config.width, config.height, mMipLevelCount, config.usageHint))
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
	return !mIsOwner;
}


VkImageLayout
ImageImpl::getPreferredImageLayout()
{
	return mPreferredImageLayout;
}


void
ImageImpl::cmdTransitionImageLayout(VkCommandBuffer commandBuffer, ImageImpl& image, VkImageLayout layout, 
	                                uint32_t firstMipLevel, uint32_t levelCount, VkAccessFlagBits srcAccessMask,
	                                VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageFlags, 
	                                VkPipelineStageFlags dstStageFlags)
{
	auto& context = image.context();
	std::vector<VkImageMemoryBarrier> barriers;
	barriers.reserve(levelCount);
	for (size_t i = 0; i < levelCount; ++i)
	{
		auto level = firstMipLevel + i;
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout                   = image.mCurrentLayout[level];
		barrier.newLayout                   = layout;
		barrier.srcQueueFamilyIndex         = context.getQueueFamilyIndex();
		barrier.dstQueueFamilyIndex         = context.getQueueFamilyIndex();
		barrier.image                       = image.getVkImage();

		image.mCurrentLayout[level] = layout;

		if (isDepthFormat(image.mFormat))
		{ 
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if (isStencilFormat(image.mFormat))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		barrier.subresourceRange.baseMipLevel   = level;
		barrier.subresourceRange.levelCount     = 1;
		barrier.subresourceRange.layerCount     = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.srcAccessMask                   = srcAccessMask;
		barrier.dstAccessMask                   = dstAccessMask;

		barriers.push_back(barrier);
	}

	vkCmdPipelineBarrier(commandBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
}
