
#include <Coral/Core.h>

#include <cassert>


uint32_t
coPixelFormatGetSizeInBytes(CoPixelFormat format)
{
    switch (format)
    {
        case CO_PIXEL_FORMAT_R_U8:             return 1;
        case CO_PIXEL_FORMAT_R_I8:             return 1;
        case CO_PIXEL_FORMAT_R_U16:            return 2;
        case CO_PIXEL_FORMAT_R_I16:            return 2;
        case CO_PIXEL_FORMAT_R_U32:            return 4;
        case CO_PIXEL_FORMAT_R_I32:            return 4;
        case CO_PIXEL_FORMAT_R_F32:            return 4;

        case CO_PIXEL_FORMAT_RG_U8:            return 2;
        case CO_PIXEL_FORMAT_RG_I8:            return 2;
        case CO_PIXEL_FORMAT_RG_U16:           return 4;
        case CO_PIXEL_FORMAT_RG_I16:           return 4;
        case CO_PIXEL_FORMAT_RG_U32:           return 8;
        case CO_PIXEL_FORMAT_RG_I32:           return 8;
        case CO_PIXEL_FORMAT_RG_F32:           return 8;

        case CO_PIXEL_FORMAT_RGB_U8:           return 3;
        case CO_PIXEL_FORMAT_RGB_I8:           return 3;
        case CO_PIXEL_FORMAT_RGB_U16:          return 6;
        case CO_PIXEL_FORMAT_RGB_I16:          return 6;
        case CO_PIXEL_FORMAT_RGB_U32:          return 12;
        case CO_PIXEL_FORMAT_RGB_I32:          return 12;
        case CO_PIXEL_FORMAT_RGB_F32:          return 12;

        case CO_PIXEL_FORMAT_RGBA_U8:          return 4;
        case CO_PIXEL_FORMAT_RGBA_I8:          return 4;
        case CO_PIXEL_FORMAT_RGBA_U16:         return 8;
        case CO_PIXEL_FORMAT_RGBA_I16:         return 8;
        case CO_PIXEL_FORMAT_RGBA_U32:         return 16;
        case CO_PIXEL_FORMAT_RGBA_I32:         return 16;
        case CO_PIXEL_FORMAT_RGBA_F32:         return 16;

        case CO_PIXEL_FORMAT_DEPTH16:          return 2;
        case CO_PIXEL_FORMAT_DEPTH24_STENCIL8: return 4;
        case CO_PIXEL_FORMAT_DEPTH_F32:        return 4;

        case CO_PIXEL_FORMAT_R_F16:            return 2;
        case CO_PIXEL_FORMAT_RG_F16:           return 4;
        case CO_PIXEL_FORMAT_RGB_F16:          return 6;
        case CO_PIXEL_FORMAT_RGBA_F16:         return 8;

        case CO_PIXEL_FORMAT_R8_SRGB:          return 2;
        case CO_PIXEL_FORMAT_RG8_SRGB:         return 4;
        case CO_PIXEL_FORMAT_RGB8_SRGB:        return 6;
        case CO_PIXEL_FORMAT_RGBA8_SRGB:       return 8;
    }

    assert(false);
    return 0;
}


uint32_t
coAttributeFormatGetSizeInBytes(CoAttributeFormat format)
{
    switch (format)
    {
        case CO_ATTRIBUTE_FORMAT_R_U16:
        case CO_ATTRIBUTE_FORMAT_R_I16:
        case CO_ATTRIBUTE_FORMAT_R_F16: 
            return 2;
        case CO_ATTRIBUTE_FORMAT_RG_U16:
        case CO_ATTRIBUTE_FORMAT_RG_I16:
        case CO_ATTRIBUTE_FORMAT_RG_F16:
            return 4;
        case CO_ATTRIBUTE_FORMAT_RGB_U16:
        case CO_ATTRIBUTE_FORMAT_RGB_I16:
        case CO_ATTRIBUTE_FORMAT_RGB_F16:
            return 6;
        case CO_ATTRIBUTE_FORMAT_RGBA_U16:
        case CO_ATTRIBUTE_FORMAT_RGBA_I16:
        case CO_ATTRIBUTE_FORMAT_RGBA_F16:
            return 8;
        case CO_ATTRIBUTE_FORMAT_R_U32:
        case CO_ATTRIBUTE_FORMAT_R_I32:
        case CO_ATTRIBUTE_FORMAT_R_F32:
            return 4;
        case CO_ATTRIBUTE_FORMAT_RG_U32:
        case CO_ATTRIBUTE_FORMAT_RG_I32:
        case CO_ATTRIBUTE_FORMAT_RG_F32:
            return 8;
        case CO_ATTRIBUTE_FORMAT_RGB_U32:
        case CO_ATTRIBUTE_FORMAT_RGB_I32:
        case CO_ATTRIBUTE_FORMAT_RGB_F32:
            return 12;
        case CO_ATTRIBUTE_FORMAT_RGBA_U32:
        case CO_ATTRIBUTE_FORMAT_RGBA_I32:
        case CO_ATTRIBUTE_FORMAT_RGBA_F32:
            return 16;
        case CO_ATTRIBUTE_FORMAT_R_U64:    
        case CO_ATTRIBUTE_FORMAT_R_I64:
        case CO_ATTRIBUTE_FORMAT_R_F64:
            return 8;
        case CO_ATTRIBUTE_FORMAT_RG_U64:
        case CO_ATTRIBUTE_FORMAT_RG_I64:
        case CO_ATTRIBUTE_FORMAT_RG_F64:
            return 16;
        case CO_ATTRIBUTE_FORMAT_RGB_U64:
        case CO_ATTRIBUTE_FORMAT_RGB_I64:
        case CO_ATTRIBUTE_FORMAT_RGB_F64:
            return 24;
        case CO_ATTRIBUTE_FORMAT_RGBA_U64:
        case CO_ATTRIBUTE_FORMAT_RGBA_I64:
        case CO_ATTRIBUTE_FORMAT_RGBA_F64:
            return 32;
    }

    assert(false);
    return 0;
}


uint32_t
coIndexFormatGetSizeInBytes(CoIndexFormat format)
{
    switch (format)
    {
        case CO_INDEX_FORMAT_UINT16: return 2;
        case CO_INDEX_FORMAT_UINT32: return 4;
    }

    assert(false);
    return 0;
}
