#ifndef CORAL_VULKAN_FRAMEBUFFERIMPL_HPP
#define CORAL_VULKAN_FRAMEBUFFERIMPL_HPP

#include <Coral/FramebufferBase.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/ImageImpl.hpp>

#include <optional>
#include <vector>


namespace Coral::Vulkan
{

class FramebufferImpl : public Coral::FramebufferBase
{
public:
	
	using FramebufferBase::FramebufferBase;

	std::optional<FramebufferCreationError> init(const Coral::FramebufferCreateConfig& config);

	ContextImpl& contextImpl() { return static_cast<ContextImpl&>(context()); }

	uint32_t width() const override;

	uint32_t height() const override;

	Coral::FramebufferSignature getSignature() override;

	const std::vector<ColorAttachment>& colorAttachments();

	const std::optional<DepthAttachment>& depthAttachment();

private:

	std::vector<ColorAttachment> mColorAttachments;

	std::optional<DepthAttachment> mDepthAttachment;

	uint32_t mWidth{ 0 };

	uint32_t mHeight{ 0 };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FRAMEBUFFERIMPL_HPP