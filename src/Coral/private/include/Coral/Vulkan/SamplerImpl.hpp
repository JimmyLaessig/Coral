#ifndef CORAL_VULKAN_SAMPLERIMPL_HPP
#define CORAL_VULKAN_SAMPLERIMPL_HPP

#include <Coral/SamplerBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/ImageImpl.hpp>

namespace Coral::Vulkan
{

class SamplerImpl : public Coral::SamplerBase
{
public:
	using SamplerBase::SamplerBase;

	virtual ~SamplerImpl();

	std::optional<Coral::SamplerCreationError> init(const Coral::SamplerCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

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