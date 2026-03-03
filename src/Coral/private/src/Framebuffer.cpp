#include <Coral/Framebuffer.h>
#include <Coral/Framebuffer.hpp>
#include <Coral/Context.hpp>
#include <Coral/Image.hpp>

#include <ranges>
#include <span>
#include <cassert>


using namespace Coral;


CoFramebuffer_T::CoFramebuffer_T(Coral::FramebufferPtr impl)
    : impl(impl)
    , layout(impl->layout())
{
}


CoResult
coContextCreateFramebuffer(CoContext context, const CoFramebufferCreateConfig* config, CoFramebuffer* framebuffer)
{
    Framebuffer::CreateConfig configImpl{};
    configImpl.depthAttachment  = config->depthAttachment ? config->depthAttachment->impl : nullptr;
    configImpl.colorAttachments = std::span(config->pColorAttachments, config->colorAttachmentCount)
        | std::views::transform([](const auto& attachment){ return Coral::ColorAttachment{ attachment.image->impl, attachment.binding }; })
        | std::ranges::to<std::vector>();

    if (auto impl = context->impl->createFramebuffer(configImpl))
    {
        *framebuffer = new CoFramebuffer_T(impl.value());
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroyFramebuffer(CoFramebuffer framebuffer)
{
    delete framebuffer;
}


void
coFramebufferGetLayout(const CoFramebuffer framebuffer, CoFramebufferLayout* layout)
{
    layout->pColorAttachments      = framebuffer->layout.colorAttachments.data();
    layout->colorAttachmentCount   = static_cast<uint32_t>(framebuffer->layout.colorAttachments.size());
    layout->depthStencilAttachment = framebuffer->layout.depthStencilAttachment ? &framebuffer->layout.depthStencilAttachment.value() : nullptr;
}
