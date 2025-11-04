#ifndef CORAL_FRAMEBUFFER_HPP
#define CORAL_FRAMEBUFFER_HPP

#include <Coral/System.hpp>
#include <Coral/CoralFwd.hpp>
#include <Coral/Types.hpp>

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace Coral
{

struct CORAL_API ColorAttachment
{
	/// The index to bind the color attachment to
	uint32_t attachment{ 0 };

	/// The image of the color attachment
	/**
	 * The image's pixel format must be a color format
	 */
	Image* image{ nullptr };
};


struct CORAL_API DepthAttachment
{
	/// The image depth attachment
	/**
	 * The image's pixel format must be a depth format
	 */
	Image* image{ nullptr };
};


struct CORAL_API FramebufferCreateConfig
{
	///
	std::span<ColorAttachment> colorAttachments;

	///
	std::optional<DepthAttachment> depthAttachment;
};

struct CORAL_API FramebufferSignature
{
	///
	std::vector<Coral::PixelFormat> colorAttachmentFormats;

	///
	std::optional<Coral::PixelFormat> depthStencilAttachmentFormat;
};

enum class FramebufferCreationError
{
	INTERNAL_ERROR,
	// Two or more color attachments are bound to the same attachment index
	DUPLICATE_COLOR_ATTACHMENTS,
	// The image format of a color attachment is invalid
	INVALID_COLOR_ATTACHMENT_FORMAT,
	// The image format of the depth-stencil attachment is invalid
	INVALID_DEPTH_STENCIL_ATTACHMENT_FORMAT
};

class CORAL_API Framebuffer
{
public:

	virtual ~Framebuffer() = default;

	virtual FramebufferSignature signature() = 0;

	virtual uint32_t width() const = 0;

	virtual uint32_t height() const = 0;

	virtual Coral::Image* colorAttachment(uint32_t attachment) = 0;
};


} // namespace Coral

#endif // !CORAL_FRAMEBUFFER_HPP