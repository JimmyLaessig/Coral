#ifndef CORAL_FRAMEBUFFER_HPP
#define CORAL_FRAMEBUFFER_HPP

#include <Coral/Framebuffer.h>
#include <Coral/CoralFwd.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace Coral
{

struct ColorAttachment
{
    /// The image of the color attachment
    /**
     * The image's pixel format must be a color format
     */
    ImagePtr image{ nullptr };

    /// The index to bind the color attachment to
    uint32_t binding{ 0 };
};


class CORAL_API Framebuffer
{
public:

    struct CreateConfig
    {
        ///
        std::vector<ColorAttachment> colorAttachments;

        ///
        ImagePtr depthAttachment;
    };

    enum class CreateError
    {
        INTERNAL_ERROR,
        // Two or more color attachments are bound to the same attachment index
        DUPLICATE_COLOR_ATTACHMENTS,
        // The image format of a color attachment is invalid
        INVALID_COLOR_ATTACHMENT_FORMAT,
        // The image format of the depth-stencil attachment is invalid
        INVALID_DEPTH_STENCIL_ATTACHMENT_FORMAT
    };

    struct Layout
    {
        ///
        std::vector<CoColorAttachmentInfo> colorAttachments;

        ///
        std::optional<CoDepthStencilAttachmentInfo> depthStencilAttachment;
    };

    virtual ~Framebuffer() = default;

    virtual Layout layout() = 0;

    virtual uint32_t width() const = 0;

    virtual uint32_t height() const = 0;
};

} // namespace Coral

struct CoFramebuffer_T
{
    CoFramebuffer_T(Coral::FramebufferPtr impl);

    Coral::FramebufferPtr impl;

    Coral::Framebuffer::Layout layout{};
};

#endif // !CORAL_FRAMEBUFFER_HPP