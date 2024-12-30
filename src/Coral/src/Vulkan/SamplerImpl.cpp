#include "SamplerImpl.hpp"

#include "VulkanWrapper.hpp"
#include "ImageImpl.hpp"

#include <cassert>
#include <tuple>


using namespace Coral::Vulkan;

//namespace
//{

VkFilter
convert(Coral::Filter magFilter)
{
	switch (magFilter)
	{
		case Coral::Filter::NEAREST:	return VK_FILTER_NEAREST;
		case Coral::Filter::LINEAR:		return VK_FILTER_LINEAR;
		default:
			assert(false);
			return {};
	}
}


//GLint
//convert(Coral::Filter minFilter, Coral::Filter mipmapFilter)
//{
//	switch (minFilter)
//	{
//		case Coral::Filter::NEAREST:
//			switch (mipmapFilter)
//			{
//				case Coral::Filter::NEAREST:	return GL_NEAREST_MIPMAP_NEAREST;
//				case Coral::Filter::LINEAR:		return GL_NEAREST_MIPMAP_LINEAR;
//				default:
//					assert(false);
//					return {};
//			}
//		case Coral::Filter::LINEAR:
//			switch (mipmapFilter)
//			{
//				case Coral::Filter::NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
//				case Coral::Filter::LINEAR:  return GL_LINEAR_MIPMAP_LINEAR;
//				default:
//					assert(false);
//					return {};
//			}
//		default:
//			assert(false);
//			return {};
//	}
//}


std::tuple<VkSamplerAddressMode, VkBorderColor>
convert(Coral::WrapMode wrapMode)
{
	switch (wrapMode)
	{
		case Coral::WrapMode::CLAMP_TO_EDGE:	return { VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
		case Coral::WrapMode::REPEAT:			return { VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
		case Coral::WrapMode::MIRROR:			return { VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
		case Coral::WrapMode::ONE:				return { VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE };
		case Coral::WrapMode::ZERO:				return { VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
		default:
			assert(false);
			return {};
	}
}
//
//} // namespace


std::optional<Coral::SamplerCreationError>
SamplerImpl::init(ContextImpl& context, const Coral::SamplerCreateConfig& config)
{
	mContext		= &context;
	mMinFilter		= config.minFilter;
	mMagFilter		= config.magFilter;
	mMipmapFilter	= config.mipmapFilter;
	mWrapMode		= config.wrapMode;

	VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	createInfo.magFilter	= convert(config.magFilter);
	createInfo.minFilter	= convert(config.minFilter);
	createInfo.mipmapMode	= config.mipmapFilter == Coral::Filter::LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;

	auto [addressMode, borderColor] = convert(config.wrapMode);

	createInfo.addressModeU = addressMode;
	createInfo.addressModeV = addressMode;
	createInfo.addressModeW = addressMode;
	createInfo.borderColor	= borderColor;
	
	createInfo.mipLodBias	= 0.f;
	createInfo.minLod		= 0.f;
	createInfo.maxLod		= VK_LOD_CLAMP_NONE;

	// TODO: Enable anisotropic filtering
	createInfo.anisotropyEnable = VK_FALSE;
	createInfo.maxAnisotropy	= 0.f;

	createInfo.compareEnable	= VK_FALSE;
	createInfo.compareOp		= VK_COMPARE_OP_ALWAYS;

	createInfo.unnormalizedCoordinates = VK_FALSE;

	if (vkCreateSampler(mContext->getVkDevice(), &createInfo, nullptr, &mSampler) != VK_SUCCESS)
	{
		return SamplerCreationError::INTERNAL_ERROR;
	}

	return {};
}


SamplerImpl::~SamplerImpl()
{
	if (mContext && mSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(mContext->getVkDevice(), mSampler, nullptr);
	}
}


VkSampler
SamplerImpl::getVkSampler()
{
	return mSampler;
}


Coral::Filter
SamplerImpl::magFilter() const
{
	return mMagFilter;
}


Coral::Filter
SamplerImpl::minFilter() const
{
	return mMinFilter;
}


Coral::Filter
SamplerImpl::mipMapFilter() const
{
	return mMipmapFilter;
}


Coral::WrapMode
SamplerImpl::wrapMode() const
{
	return mWrapMode;
}