#ifndef CORAL_VULKAN_FRAMEBUFFERIMPL_HPP
#define CORAL_VULKAN_FRAMEBUFFERIMPL_HPP

#include <Coral/Framebuffer.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

#include <optional>
#include <vector>


namespace Coral::Vulkan
{

class FramebufferImpl : public Coral::Framebuffer
	                  , public Resource
	                  , public std::enable_shared_from_this<FramebufferImpl>
{
public:
	
	using Resource::Resource;

	std::optional<FramebufferCreationError> init(const Coral::FramebufferCreateConfig& config);

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