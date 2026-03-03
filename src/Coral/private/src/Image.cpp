#include <Coral/Image.h>
#include <Coral/Image.hpp>

#include <Coral/Context.hpp>

using namespace Coral;

CoResult
coContextCreateImage(CoContext context, const CoImageCreateConfig* pConfig, CoImage* pImage)
{
    auto impl = context->impl->createImage(*pConfig);
    if (impl)
    {
        *pImage = new CoImage_T{ impl.value() };
        return CO_SUCCESS;
    }

    return static_cast<CoResult>(impl.error());
}


void
coDestroyImage(CoImage image)
{
    delete image;
}


void
coImageGetExtent(const CoImage image, CoExtent* pExtent)
{
    pExtent->width  = image->impl->width();
    pExtent->height = image->impl->height();
}


CoPixelFormat
coImageGetPixelFormat(const CoImage image)
{
    return image->impl->format();
}


uint32_t
coImageGetMipLevelCount(const CoImage image)
{
    return image->impl->getMipLevels();
}
