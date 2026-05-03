#include <Coral/Vulkan/SamplerImpl.hpp>

#include <cassert>
#include <tuple>


using namespace Coral::Vulkan;

namespace
{

VkFilter
convert(CoFilter magFilter)
{
    switch (magFilter)
    {
        case CO_FILTER_NEAREST:    return VK_FILTER_NEAREST;
        case CO_FILTER_LINEAR:  return VK_FILTER_LINEAR;
        default:
            assert(false);
            return {};
    }
}


std::tuple<VkSamplerAddressMode, VkBorderColor>
convert(CoWrapMode wrapMode)
{
    switch (wrapMode)
    {
        case CO_WRAP_MODE_CLAMP_TO_EDGE: return { VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
        case CO_WRAP_MODE_REPEAT:         return { VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
        case CO_WRAP_MODE_MIRROR:         return { VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
        case CO_WRAP_MODE_ONE:             return { VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE };
        case CO_WRAP_MODE_ZERO:             return { VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK };
        default:
            assert(false);
            return {};
    }
}

} // namespace


SamplerImpl::~SamplerImpl()
{
    if (mSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(context().getVkDevice(), mSampler, nullptr);
    }
}


std::optional<Coral::Sampler::CreateError>
SamplerImpl::init(const Coral::Sampler::CreateConfig& config)
{
    mMinFilter        = config.minFilter;
    mMagFilter        = config.magFilter;
    mMipmapFilter    = config.mipmapFilter;
    mWrapMode        = config.wrapMode;

    VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    createInfo.magFilter    = convert(config.magFilter);
    createInfo.minFilter    = convert(config.minFilter);
    createInfo.mipmapMode    = config.mipmapFilter == CO_FILTER_LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;

    auto [addressMode, borderColor] = convert(config.wrapMode);

    createInfo.addressModeU = addressMode;
    createInfo.addressModeV = addressMode;
    createInfo.addressModeW = addressMode;
    createInfo.borderColor    = borderColor;
    
    createInfo.mipLodBias    = 0.f;
    createInfo.minLod        = 0.f;
    createInfo.maxLod        = VK_LOD_CLAMP_NONE;

    // TODO: Enable anisotropic filtering
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.maxAnisotropy    = 0.f;

    createInfo.compareEnable    = VK_FALSE;
    createInfo.compareOp        = VK_COMPARE_OP_ALWAYS;

    createInfo.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(context().getVkDevice(), &createInfo, nullptr, &mSampler) != VK_SUCCESS)
    {
        return Sampler::CreateError::INTERNAL_ERROR;
    }

    return {};
}


VkSampler
SamplerImpl::getVkSampler()
{
    return mSampler;
}


CoFilter
SamplerImpl::magFilter() const
{
    return mMagFilter;
}


CoFilter
SamplerImpl::minFilter() const
{
    return mMinFilter;
}


CoFilter
SamplerImpl::mipMapFilter() const
{
    return mMipmapFilter;
}


CoWrapMode
SamplerImpl::wrapMode() const
{
    return mWrapMode;
}