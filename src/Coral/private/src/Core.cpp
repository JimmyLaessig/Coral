
#include <Coral/Core.h>

#include <cassert>


uint32_t
coPixelFormatGetSizeInBytes(CoPixelFormat format)
{
    switch (format)
    {
        case CO_PIXEL_FORMAT_R8_UI:            return 1;
        case CO_PIXEL_FORMAT_R8_I:             return 1;
        case CO_PIXEL_FORMAT_R16_UI:           return 2;
        case CO_PIXEL_FORMAT_R16_I:            return 2;
        case CO_PIXEL_FORMAT_R32_UI:           return 4;
        case CO_PIXEL_FORMAT_R32_I:            return 4;
        case CO_PIXEL_FORMAT_R32_F:            return 4;

        case CO_PIXEL_FORMAT_RG8_UI:           return 2;
        case CO_PIXEL_FORMAT_RG8_I:            return 2;
        case CO_PIXEL_FORMAT_RG16_UI:          return 4;
        case CO_PIXEL_FORMAT_RG16_I:           return 4;
        case CO_PIXEL_FORMAT_RG32_UI:          return 8;
        case CO_PIXEL_FORMAT_RG32_I:           return 8;
        case CO_PIXEL_FORMAT_RG32_F:           return 8;

        case CO_PIXEL_FORMAT_RGB8_UI:          return 3;
        case CO_PIXEL_FORMAT_RGB8_I:           return 3;
        case CO_PIXEL_FORMAT_RGB16_UI:         return 6;
        case CO_PIXEL_FORMAT_RGB16_I:          return 6;
        case CO_PIXEL_FORMAT_RGB32_UI:         return 12;
        case CO_PIXEL_FORMAT_RGB32_I:          return 12;
        case CO_PIXEL_FORMAT_RGB32_F:          return 12;

        case CO_PIXEL_FORMAT_RGBA8_UI:         return 4;
        case CO_PIXEL_FORMAT_RGBA8_I:          return 4;
        case CO_PIXEL_FORMAT_RGBA16_UI:        return 8;
        case CO_PIXEL_FORMAT_RGBA16_I:         return 8;
        case CO_PIXEL_FORMAT_RGBA32_UI:        return 16;
        case CO_PIXEL_FORMAT_RGBA32_I:         return 16;
        case CO_PIXEL_FORMAT_RGBA32_F:         return 16;

        case CO_PIXEL_FORMAT_DEPTH16:          return 2;
        case CO_PIXEL_FORMAT_DEPTH24_STENCIL8: return 4;
        case CO_PIXEL_FORMAT_DEPTH32_F:        return 4;

        case CO_PIXEL_FORMAT_R16_F:            return 2;
        case CO_PIXEL_FORMAT_RG16_F:           return 4;
        case CO_PIXEL_FORMAT_RGB16_F:          return 6;
        case CO_PIXEL_FORMAT_RGBA16_F:         return 8;

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
        case CO_ATTRIBUTE_FORMAT_UINT16: return 2;
        case CO_ATTRIBUTE_FORMAT_UINT32: return 4;
        case CO_ATTRIBUTE_FORMAT_INT16:  return 2;
        case CO_ATTRIBUTE_FORMAT_INT32:  return 4;
        case CO_ATTRIBUTE_FORMAT_FLOAT:  return 4;
        case CO_ATTRIBUTE_FORMAT_VEC2F:  return 8;
        case CO_ATTRIBUTE_FORMAT_VEC3F:  return 12;
        case CO_ATTRIBUTE_FORMAT_VEC4F:  return 16;
    }

    assert(false);
    return 0;
}


uint32_t
coUniformFormatGetSizeInBytes(CoUniformFormat format)
{
    //    switch (format)
    //{
    //case UniformFormat::BOOL:    return 4;
    //case UniformFormat::INT32:    return 4;
    //case UniformFormat::FLOAT:    return 4;
    //case UniformFormat::VEC2F:    return 8;
    //case UniformFormat::VEC3F:    return 12;
    //case UniformFormat::VEC4F:    return 16;
    //case UniformFormat::VEC2I:    return 8;
    //case UniformFormat::VEC3I:    return 12;
    //case UniformFormat::VEC4I:    return 16;
    //case UniformFormat::MAT33F:    return 36;
    //case UniformFormat::MAT44F:    return 64;
    //};

    //std::unreachable();
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
