#ifndef CORAL_VULKAN_FRAMEBUFFERIMPL_HPP
#define CORAL_VULKAN_FRAMEBUFFERIMPL_HPP

#include <Coral/Framebuffer.hpp>

#include "ContextImpl.hpp"

#include <optional>
#include <vector>


namespace Coral::Vulkan
{

class FramebufferImpl : public Coral::Framebuffer
{
public:

	std::optional<FramebufferCreationError> init(Coral::Vulkan::ContextImpl& context, const Coral::FramebufferCreateConfig& config);

	const std::vector<ColorAttachment>& colorAttachments();

	const std::optional<DepthAttachment>& depthAttachment();

	uint32_t width();

	uint32_t height();

	Coral::FramebufferSignature getSignature() override;

private:

	Coral::Vulkan::ContextImpl* mContext{ nullptr };

	std::vector<ColorAttachment> mColorAttachments;

	std::optional<DepthAttachment> mDepthAttachment;

	uint32_t mWidth{ 0 };

	uint32_t mHeight{ 0 };

};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FRAMEBUFFERIMPL_HPP