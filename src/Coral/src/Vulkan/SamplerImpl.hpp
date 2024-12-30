#ifndef CORAL_VULKAN_SAMPLERIMPL_HPP
#define CORAL_VULKAN_SAMPLERIMPL_HPP

#include <Coral/Sampler.hpp>

#include "ContextImpl.hpp"

#include "ImageImpl.hpp"

namespace Coral::Vulkan
{

class SamplerImpl : public Coral::Sampler
{
public:

	std::optional<Coral::SamplerCreationError> init(Coral::Vulkan::ContextImpl& context, const Coral::SamplerCreateConfig& config);

	virtual ~SamplerImpl();
	
	VkSampler getVkSampler();

	Filter magFilter() const override;
	
	Filter minFilter() const override;
	
	Filter mipMapFilter() const override;
	
	WrapMode wrapMode() const override;

private:
	
	ContextImpl* mContext{ nullptr };

	Coral::Filter mMinFilter{ Coral::Filter::NEAREST };

	Coral::Filter mMagFilter{ Coral::Filter::NEAREST };

	Coral::Filter mMipmapFilter{ Coral::Filter::NEAREST };

	Coral::WrapMode mWrapMode{ Coral::WrapMode::REPEAT };

	VkSampler mSampler{ VK_NULL_HANDLE };
};

} // namespace Coral::Vulkan

#endif // ! CORAL_VULKAN_SAMPLERIMPL_HPP