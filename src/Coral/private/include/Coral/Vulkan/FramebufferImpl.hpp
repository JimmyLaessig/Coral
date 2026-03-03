#ifndef CORAL_VULKAN_FRAMEBUFFERIMPL_HPP
#define CORAL_VULKAN_FRAMEBUFFERIMPL_HPP

#include <Coral/Framebuffer.hpp>

#include <Coral/Vulkan/Fwd.hpp>
#include <Coral/Vulkan/Resource.hpp>
#include <Coral/Vulkan/Vulkan.hpp>

#include <optional>
#include <map>


namespace Coral::Vulkan
{

class FramebufferImpl : public Coral::Framebuffer
	                  , public Resource
	                  , public std::enable_shared_from_this<FramebufferImpl>
{
public:
	
	using Resource::Resource;

	std::optional<Framebuffer::CreateError> init(const Coral::Framebuffer::CreateConfig& config);

	uint32_t width() const override;

	uint32_t height() const override;

	Coral::Framebuffer::Layout layout() override;

	const std::map<uint32_t, ImageImplPtr>& colorAttachments();

	ImageImplPtr depthAttachment();

private:

	std::map<uint32_t, ImageImplPtr> mColorAttachments;

	ImageImplPtr mDepthAttachment;

	uint32_t mWidth;
	uint32_t mHeight;
};

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FRAMEBUFFERIMPL_HPP