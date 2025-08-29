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

	std::optional<Coral::SamplerCreationError> init(const Coral::SamplerCreateConfig& config);

	Filter magFilter() const override;

	Filter minFilter() const override;

	Filter mipMapFilter() const override;

	WrapMode wrapMode() const override;

	VkSampler getVkSampler();

private:

	Coral::Filter mMinFilter{ Coral::Filter::NEAREST };

	Coral::Filter mMagFilter{ Coral::Filter::NEAREST };

	Coral::Filter mMipmapFilter{ Coral::Filter::NEAREST };

	Coral::WrapMode mWrapMode{ Coral::WrapMode::REPEAT };

	VkSampler mSampler{ VK_NULL_HANDLE };
};

} // namespace Coral::Vulkan

#endif // ! CORAL_VULKAN_SAMPLERIMPL_HPP
