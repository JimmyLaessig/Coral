#ifndef CORAL_FRAMEBUFFER_HPP
#define CORAL_FRAMEBUFFER_HPP

#include <Coral/System.hpp>

#include <Coral/Image.hpp>
#include <Coral/Types.hpp>

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace Coral
{

/// Specifies how content of an attachment are initialized at the beginning of a render pass.
enum class ClearOp
{
	/// Specifies that the content of the image will be cleared.
	CLEAR,
	/// Specifies that the previous content of the image will be be preserved the initial values.
	LOAD,
	/// Specifies that the previous content of the image need not be preserved.
	DONT_CARE,
};


struct CORAL_API ColorAttachment
{
	/// The index to bind the color attachment to
	// uint32_t attachment{ 0 };

	/// The image of the color attachment
	/**
	 * The image's pixel format must be a color format
	 */
	Image* image{ nullptr };

	/// Clear operation
	ClearOp clearOp{ ClearOp::DONT_CARE };
};


struct CORAL_API DepthAttachment
{
	/// The image depth attachment
	/**
	 * The image's pixel format must be a depth format
	 */
	Image* image{ nullptr };

	/// Clear operation
	ClearOp clearOp{ ClearOp::DONT_CARE };
};


struct CORAL_API FramebufferCreateConfig
{
	///
	std::span<ColorAttachment> colorAttachment;

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
	INTERNAL_ERROR
};

class CORAL_API Framebuffer
{
public:

	virtual ~Framebuffer() = default;

	virtual FramebufferSignature getSignature() = 0;
};


} // namespace Coral

#endif // !CORAL_FRAMEBUFFER_HPP