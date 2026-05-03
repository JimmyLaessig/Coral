#include <Coral/Vulkan/FramebufferImpl.hpp>

#include <Coral/Vulkan/VulkanFormat.hpp>

#include <Coral/Vulkan/ImageImpl.hpp>

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

VkAttachmentLoadOp
convert(CoClearOp clearOp)
{
    switch (clearOp)
    {
        case CO_CLEAR_OP_DONT_CARE: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case CO_CLEAR_OP_LOAD:      return VK_ATTACHMENT_LOAD_OP_LOAD;
        case CO_CLEAR_OP_CLEAR:     return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:
            assert(false);
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }
}


std::optional<VkExtent2D>
getFramebufferExtent(const Coral::Framebuffer::CreateConfig& config)
{
    if (!config.colorAttachments.empty())
    {
        return VkExtent2D{ config.colorAttachments.front().image->width(), config.colorAttachments.front().image->height() };
    }
    else if (config.depthAttachment)
    {
        return VkExtent2D{ config.depthAttachment->width(), config.depthAttachment->height() };
    }

    return {};
}

} // namespace


std::optional<Coral::Framebuffer::CreateError>
FramebufferImpl::init(const Coral::Framebuffer::CreateConfig& config)
{
    auto extent = getFramebufferExtent(config);

    if (!extent)
    {
        return Framebuffer::CreateError::INTERNAL_ERROR;
    }

    mWidth  = extent->width;
    mHeight = extent->height;

    for (const auto& [image, binding] : config.colorAttachments)
    {
        mColorAttachments.emplace(binding, std::static_pointer_cast<ImageImpl>(image));
    }

    if (mColorAttachments.size() != config.colorAttachments.size())
    {
        return Framebuffer::CreateError::DUPLICATE_COLOR_ATTACHMENTS;
    }

    mDepthAttachment = std::static_pointer_cast<ImageImpl>(config.depthAttachment);

    // Validate the attachments:
    // 1. The image format must not be a depth format
    // 2. The attachment index must be unique
    // 3. The depth attachment format must be a depth format

    bool colorFormatsValid = std::ranges::none_of(config.colorAttachments, [](const auto& attachment)
    {
        return isDepthFormat(attachment.image->format());
    });

    if (!colorFormatsValid)
    {
        return Framebuffer::CreateError::INVALID_COLOR_ATTACHMENT_FORMAT;
    }

    if (mDepthAttachment)
    {
        if (!isDepthFormat(mDepthAttachment->format()))
        {
            return Framebuffer::CreateError::INVALID_DEPTH_STENCIL_ATTACHMENT_FORMAT;
        }
    }

    return {};
}


const std::map<uint32_t, ImageImplPtr>&
FramebufferImpl::colorAttachments()
{
    return mColorAttachments;
}

ImageImplPtr
FramebufferImpl::depthAttachment()
{
    return mDepthAttachment;
}


uint32_t
FramebufferImpl::width() const
{
    return mWidth;
}


uint32_t
FramebufferImpl::height() const
{
    return mHeight;
}


Coral::Framebuffer::Layout
FramebufferImpl::layout()
{
    Coral::Framebuffer::Layout layout{};

    if (mDepthAttachment)
    {
        layout.depthStencilAttachment = CoDepthStencilAttachmentInfo{ mDepthAttachment->format() };
    }

    for (const auto& [binding, image] : mColorAttachments)
    {
        layout.colorAttachments.push_back({ image->format(), binding });
    }

    return layout;
}
