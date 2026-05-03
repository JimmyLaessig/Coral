#ifndef CORAL_VULKAN_SAMPLERIMPL_HPP
#define CORAL_VULKAN_SAMPLERIMPL_HPP

#include <Coral/Sampler.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

namespace Coral::Vulkan
{

class SamplerImpl : public Coral::Sampler
                  , public Resource
                  , public std::enable_shared_from_this<SamplerImpl>
{
public:
    using Resource::Resource;

    virtual ~SamplerImpl();

    std::optional<Coral::Sampler::CreateError> init(const Coral::Sampler::CreateConfig& config);

    CoFilter magFilter() const override;

    CoFilter minFilter() const override;

    CoFilter mipMapFilter() const override;

    CoWrapMode wrapMode() const override;

    VkSampler getVkSampler();

private:

    CoFilter mMinFilter{ CO_FILTER_NEAREST };

    CoFilter mMagFilter{ CO_FILTER_NEAREST };

    CoFilter mMipmapFilter{ CO_FILTER_NEAREST };

    CoWrapMode mWrapMode{ CO_WRAP_MODE_REPEAT };

    VkSampler mSampler{ VK_NULL_HANDLE };
};

} // namespace Coral::Vulkan

#endif // ! CORAL_VULKAN_SAMPLERIMPL_HPP
