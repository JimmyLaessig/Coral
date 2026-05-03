#ifndef CORAL_IMAGE_H
#define CORAL_IMAGE_H

#include <Coral/Context.h>


typedef enum
{
    // The image is primarily used as read-only resource in a shader
    CO_IMAGE_USAGE_HINT_SHADER_READ_ONLY       = 0,
    // The image is primarily used as frame buffer attachment
    CO_IMAGE_USAGE_HINT_FRAMEBUFFER_ATTACHMENT = 1,

} CoImageUsageHint;


/// Configuration to create an image
typedef struct
{
    /// The extent of the image
    CoExtent extent;

    /// Flag indicating if the image should be equipped with multiple levels of detail for minified sampling of the image.
    bool hasMipMaps;

    /// The format and type of the texel blocks contained in the Image
    CoPixelFormat format;

    /// Hint about the usage of the image
    /**
     * The usage hint helps Coral to optimize memory layout of the image for best performance. Note that the hint is
     * not binding and images with usage hint SHADER_READ_ONLY can still be used as framebuffer attachments and vice
     * versa.
     */
    CoImageUsageHint usageHint;
} CoImageCreateConfig;


struct CoImage_T;

typedef CoImage_T* CoImage;


CORAL_API CoResult coContextCreateImage(CoContext context, const CoImageCreateConfig* pConfig, CoImage* pImage);

CORAL_API void coDestroyImage(CoImage image);

/// Get the width and height of the image
CORAL_API void coImageGetExtent(const CoImage image, CoExtent* pExtent);

/// Get the pixel format of the Image
CORAL_API CoPixelFormat coImageGetPixelFormat(const CoImage image);

/// Get the number of mipmap levels
CORAL_API uint32_t coImageGetMipLevelCount(const CoImage image);

#endif // !CORAL_IMAGE_HPP
