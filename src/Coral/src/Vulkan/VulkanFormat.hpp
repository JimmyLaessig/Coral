#ifndef CORAL_VULKAN_FORMAT_HPP
#define CORAL_VULKAN_FORMAT_HPP

#include "Vulkan.hpp"

#include <Coral/Buffer.h>

#include <utility>

namespace Coral::Vulkan
{

inline VkFormat
convert(CoPixelFormat format)
{
    switch (format)
    {
        case CO_PIXEL_FORMAT_DEPTH_F32:        return VK_FORMAT_D32_SFLOAT;
        case CO_PIXEL_FORMAT_DEPTH24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case CO_PIXEL_FORMAT_DEPTH16:          return VK_FORMAT_D16_UNORM;

        case CO_PIXEL_FORMAT_RGBA_F32:         return VK_FORMAT_R32G32B32A32_SFLOAT;
        case CO_PIXEL_FORMAT_RGB_F32:          return VK_FORMAT_R32G32B32_SFLOAT;
        case CO_PIXEL_FORMAT_RG_F32:           return VK_FORMAT_R32G32_SFLOAT;
        case CO_PIXEL_FORMAT_R_F32:            return VK_FORMAT_R32_SFLOAT;

        case CO_PIXEL_FORMAT_RGBA_F16:         return VK_FORMAT_R16G16B16A16_SFLOAT;
        case CO_PIXEL_FORMAT_RGB_F16:          return VK_FORMAT_R16G16B16_SFLOAT;
        case CO_PIXEL_FORMAT_RG_F16:           return VK_FORMAT_R16G16_SFLOAT;
        case CO_PIXEL_FORMAT_R_F16:            return VK_FORMAT_R16_SFLOAT;

        case CO_PIXEL_FORMAT_RGBA_I32:         return VK_FORMAT_R32G32B32A32_SINT;
        case CO_PIXEL_FORMAT_RGB_I32:          return VK_FORMAT_R32G32B32_SINT;
        case CO_PIXEL_FORMAT_RG_I32:           return VK_FORMAT_R32G32_SINT;
        case CO_PIXEL_FORMAT_R_I32:            return VK_FORMAT_R32_SINT;

        case CO_PIXEL_FORMAT_RGBA_U32:         return VK_FORMAT_R32G32B32A32_UINT;
        case CO_PIXEL_FORMAT_RGB_U32:          return VK_FORMAT_R32G32B32_UINT;
        case CO_PIXEL_FORMAT_RG_U32:           return VK_FORMAT_R32G32_UINT;
        case CO_PIXEL_FORMAT_R_U32:            return VK_FORMAT_R32_UINT;

        case CO_PIXEL_FORMAT_RGBA_I16:         return VK_FORMAT_R16G16B16A16_SNORM;
        case CO_PIXEL_FORMAT_RGB_I16:          return VK_FORMAT_R16G16B16_SNORM;
        case CO_PIXEL_FORMAT_RG_I16:           return VK_FORMAT_R16G16_SNORM;
        case CO_PIXEL_FORMAT_R_I16:            return VK_FORMAT_R16_SNORM;

        case CO_PIXEL_FORMAT_RGBA_U16:         return VK_FORMAT_R16G16B16A16_UNORM;
        case CO_PIXEL_FORMAT_RGB_U16:          return VK_FORMAT_R16G16B16_UNORM;
        case CO_PIXEL_FORMAT_RG_U16:           return VK_FORMAT_R16G16_UNORM;
        case CO_PIXEL_FORMAT_R_U16:            return VK_FORMAT_R16_UNORM;

        case CO_PIXEL_FORMAT_RGBA_I8:          return VK_FORMAT_R8G8B8A8_SNORM;
        case CO_PIXEL_FORMAT_RGB_I8:           return VK_FORMAT_R8G8B8_SNORM;
        case CO_PIXEL_FORMAT_RG_I8:            return VK_FORMAT_R8G8_SNORM;
        case CO_PIXEL_FORMAT_R_I8:             return VK_FORMAT_R8_UNORM;

        case CO_PIXEL_FORMAT_RGBA_U8:         return VK_FORMAT_R8G8B8A8_UNORM;
        case CO_PIXEL_FORMAT_RGB_U8:          return VK_FORMAT_R8G8B8_UNORM;
        case CO_PIXEL_FORMAT_RG_U8:           return VK_FORMAT_R8G8_UNORM;
        case CO_PIXEL_FORMAT_R_U8:            return VK_FORMAT_R8_UNORM;

        case CO_PIXEL_FORMAT_RGBA8_SRGB:       return VK_FORMAT_R8G8B8A8_SRGB;
        case CO_PIXEL_FORMAT_RGB8_SRGB:        return VK_FORMAT_R8G8B8_SRGB;
        case CO_PIXEL_FORMAT_RG8_SRGB:         return VK_FORMAT_R8G8_SRGB;
        case CO_PIXEL_FORMAT_R8_SRGB:          return VK_FORMAT_R8_SRGB;
    }

    std::unreachable();
};


inline CoPixelFormat
convert(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_D32_SFLOAT:          return CO_PIXEL_FORMAT_DEPTH_F32;
        case VK_FORMAT_D24_UNORM_S8_UINT:   return CO_PIXEL_FORMAT_DEPTH24_STENCIL8;
        case VK_FORMAT_D16_UNORM:           return CO_PIXEL_FORMAT_DEPTH16;

        case VK_FORMAT_R32G32B32A32_SFLOAT: return CO_PIXEL_FORMAT_RGBA_F32;
        case VK_FORMAT_R32G32B32_SFLOAT:    return CO_PIXEL_FORMAT_RGB_F32;
        case VK_FORMAT_R32G32_SFLOAT:       return CO_PIXEL_FORMAT_RG_F32;
        case VK_FORMAT_R32_SFLOAT:          return CO_PIXEL_FORMAT_R_F32;

        case VK_FORMAT_R16G16B16A16_SFLOAT: return CO_PIXEL_FORMAT_RGBA_F16;
        case VK_FORMAT_R16G16B16_SFLOAT:    return CO_PIXEL_FORMAT_RGB_F16;
        case VK_FORMAT_R16G16_SFLOAT:       return CO_PIXEL_FORMAT_RG_F16;
        case VK_FORMAT_R16_SFLOAT:          return CO_PIXEL_FORMAT_R_F16;

        case VK_FORMAT_R32G32B32A32_SINT:   return CO_PIXEL_FORMAT_RGBA_I32;
        case VK_FORMAT_R32G32B32_SINT:      return CO_PIXEL_FORMAT_RGB_I32;
        case VK_FORMAT_R32G32_SINT:         return CO_PIXEL_FORMAT_RG_I32;
        case VK_FORMAT_R32_SINT:            return CO_PIXEL_FORMAT_R_I32;

        case VK_FORMAT_R32G32B32A32_UINT:   return CO_PIXEL_FORMAT_RGBA_U32;
        case VK_FORMAT_R32G32B32_UINT:      return CO_PIXEL_FORMAT_RGB_U32;
        case VK_FORMAT_R32G32_UINT:         return CO_PIXEL_FORMAT_RG_U32;
        case VK_FORMAT_R32_UINT:            return CO_PIXEL_FORMAT_R_U32;

        case VK_FORMAT_R16G16B16A16_SINT:   return CO_PIXEL_FORMAT_RGBA_I16;
        case VK_FORMAT_R16G16B16_SINT:      return CO_PIXEL_FORMAT_RGB_I16;
        case VK_FORMAT_R16G16_SINT:         return CO_PIXEL_FORMAT_RG_I16;
        case VK_FORMAT_R16_SINT:            return CO_PIXEL_FORMAT_R_I16;

        case VK_FORMAT_R16G16B16A16_UINT:   return CO_PIXEL_FORMAT_RGBA_U16;
        case VK_FORMAT_R16G16B16_UINT:      return CO_PIXEL_FORMAT_RGB_U16;
        case VK_FORMAT_R16G16_UINT:         return CO_PIXEL_FORMAT_RG_U16;
        case VK_FORMAT_R16_UINT:            return CO_PIXEL_FORMAT_R_U16;

        case VK_FORMAT_R8G8B8A8_SINT:       return CO_PIXEL_FORMAT_RGBA_I8;
        case VK_FORMAT_R8G8B8_SINT:         return CO_PIXEL_FORMAT_RGB_I8;
        case VK_FORMAT_R8G8_SINT:           return CO_PIXEL_FORMAT_RG_I8;
        case VK_FORMAT_R8_SINT:             return CO_PIXEL_FORMAT_R_I8;

        case VK_FORMAT_R8G8B8A8_UNORM:       return CO_PIXEL_FORMAT_RGBA_U8;
        case VK_FORMAT_R8G8B8_UNORM:         return CO_PIXEL_FORMAT_RGB_U8;
        case VK_FORMAT_R8G8_UNORM:           return CO_PIXEL_FORMAT_RG_U8;
        case VK_FORMAT_R8_UNORM:             return CO_PIXEL_FORMAT_R_U8;
                                            
        case VK_FORMAT_R8G8B8A8_SRGB:       return CO_PIXEL_FORMAT_RGBA8_SRGB;
        case VK_FORMAT_R8G8B8_SRGB:         return CO_PIXEL_FORMAT_RGB8_SRGB;
        case VK_FORMAT_R8G8_SRGB:           return CO_PIXEL_FORMAT_RG8_SRGB;
        case VK_FORMAT_R8_SRGB:             return CO_PIXEL_FORMAT_R8_SRGB;

        default:
            std::unreachable();
    }
};


inline bool
isDepthFormat(CoPixelFormat format)
{
    switch (format)
    {
        case CO_PIXEL_FORMAT_DEPTH_F32:            
        case CO_PIXEL_FORMAT_DEPTH24_STENCIL8:    
        case CO_PIXEL_FORMAT_DEPTH16:            
            return true;
        default:
            return false;
    }
}


inline bool
isStencilFormat(CoPixelFormat format)
{
    switch (format)
    {
        case CO_PIXEL_FORMAT_DEPTH24_STENCIL8:
            return true;
        default:
            return false;
    }
}


inline VkBufferUsageFlags
convert(CoBufferType bufferType)
{
    
    switch (bufferType)
    {
        case CO_BUFFER_TYPE_INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        case CO_BUFFER_TYPE_VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        case CO_BUFFER_TYPE_STORAGE:
            return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        case CO_BUFFER_TYPE_UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        case CO_BUFFER_TYPE_INDIRECT:
            return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }

    std::unreachable();    
}

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FORMAT_HPP