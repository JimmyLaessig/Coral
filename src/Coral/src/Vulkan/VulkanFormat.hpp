#ifndef CORAL_VULKAN_FORMAT_HPP
#define CORAL_VULKAN_FORMAT_HPP

#include "Vulkan.hpp"
	
#include <Coral/Types.hpp>
#include <Coral/Buffer.hpp>

#include <utility>

namespace Coral::Vulkan
{

inline VkFormat
convert(Coral::PixelFormat format)
{
	switch (format)
	{
		case Coral::PixelFormat::DEPTH32_F:			return VK_FORMAT_D32_SFLOAT;
		case Coral::PixelFormat::DEPTH24_STENCIL8:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case Coral::PixelFormat::DEPTH16:			return VK_FORMAT_D16_UNORM;

		case Coral::PixelFormat::RGBA32_F:			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Coral::PixelFormat::RGB32_F:			return VK_FORMAT_R32G32B32_SFLOAT;
		case Coral::PixelFormat::RG32_F:			return VK_FORMAT_R32G32_SFLOAT;
		case Coral::PixelFormat::R32_F:				return VK_FORMAT_R32_SFLOAT;

		case Coral::PixelFormat::RGBA16_F:			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Coral::PixelFormat::RGB16_F:			return VK_FORMAT_R16G16B16_SFLOAT;
		case Coral::PixelFormat::RG16_F:			return VK_FORMAT_R16G16_SFLOAT;
		case Coral::PixelFormat::R16_F:				return VK_FORMAT_R16_SFLOAT;

		case Coral::PixelFormat::RGBA32_I:			return VK_FORMAT_R32G32B32A32_SINT;
		case Coral::PixelFormat::RGB32_I:			return VK_FORMAT_R32G32B32_SINT;
		case Coral::PixelFormat::RG32_I:			return VK_FORMAT_R32G32_SINT;
		case Coral::PixelFormat::R32_I:				return VK_FORMAT_R32_SINT;

		case Coral::PixelFormat::RGBA32_UI:			return VK_FORMAT_R32G32B32A32_UINT;
		case Coral::PixelFormat::RGB32_UI:			return VK_FORMAT_R32G32B32_UINT;
		case Coral::PixelFormat::RG32_UI:			return VK_FORMAT_R32G32_UINT;
		case Coral::PixelFormat::R32_UI:			return VK_FORMAT_R32_UINT;

		case Coral::PixelFormat::RGBA16_I: 			return VK_FORMAT_R16G16B16A16_SNORM;
		case Coral::PixelFormat::RGB16_I:  			return VK_FORMAT_R16G16B16_SNORM;
		case Coral::PixelFormat::RG16_I:   			return VK_FORMAT_R16G16_SNORM;
		case Coral::PixelFormat::R16_I:	   			return VK_FORMAT_R16_SNORM;

		case Coral::PixelFormat::RGBA16_UI:			return VK_FORMAT_R16G16B16A16_UNORM;
		case Coral::PixelFormat::RGB16_UI:			return VK_FORMAT_R16G16B16_UNORM;
		case Coral::PixelFormat::RG16_UI:			return VK_FORMAT_R16G16_UNORM;
		case Coral::PixelFormat::R16_UI:			return VK_FORMAT_R16_UNORM;

		case Coral::PixelFormat::RGBA8_I:			return VK_FORMAT_R8G8B8A8_SNORM;
		case Coral::PixelFormat::RGB8_I:			return VK_FORMAT_R8G8B8_SNORM;
		case Coral::PixelFormat::RG8_I:				return VK_FORMAT_R8G8_SNORM;
		case Coral::PixelFormat::R8_I:				return VK_FORMAT_R8_UNORM;

		case Coral::PixelFormat::RGBA8_UI:			return VK_FORMAT_R8G8B8A8_UNORM;
		case Coral::PixelFormat::RGB8_UI:			return VK_FORMAT_R8G8B8_UNORM;
		case Coral::PixelFormat::RG8_UI:			return VK_FORMAT_R8G8_UNORM;
		case Coral::PixelFormat::R8_UI:				return VK_FORMAT_R8_UNORM;

		case Coral::PixelFormat::RGBA8_SRGB:		return VK_FORMAT_R8G8B8A8_SRGB;
		case Coral::PixelFormat::RGB8_SRGB:			return VK_FORMAT_R8G8B8_SRGB;
		case Coral::PixelFormat::RG8_SRGB:			return VK_FORMAT_R8G8_SRGB;
		case Coral::PixelFormat::R8_SRGB:			return VK_FORMAT_R8_SRGB;
	}

	std::unreachable();
};


inline Coral::PixelFormat
convert(VkFormat format)
{
	switch (format)
	{
		case VK_FORMAT_D32_SFLOAT:					return Coral::PixelFormat::DEPTH32_F;
		case VK_FORMAT_D24_UNORM_S8_UINT:			return Coral::PixelFormat::DEPTH24_STENCIL8;
		case VK_FORMAT_D16_UNORM:					return Coral::PixelFormat::DEPTH16;

		case VK_FORMAT_R32G32B32A32_SFLOAT:	   		return Coral::PixelFormat::RGBA32_F;
		case VK_FORMAT_R32G32B32_SFLOAT:		   	return Coral::PixelFormat::RGB32_F;
		case VK_FORMAT_R32G32_SFLOAT:			   	return Coral::PixelFormat::RG32_F;
		case VK_FORMAT_R32_SFLOAT:			   		return Coral::PixelFormat::R32_F;

		case VK_FORMAT_R16G16B16A16_SFLOAT:	 		return Coral::PixelFormat::RGBA16_F;
		case VK_FORMAT_R16G16B16_SFLOAT:		 	return Coral::PixelFormat::RGB16_F;
		case VK_FORMAT_R16G16_SFLOAT:			 	return Coral::PixelFormat::RG16_F;
		case VK_FORMAT_R16_SFLOAT:			 		return Coral::PixelFormat::R16_F;

		case VK_FORMAT_R32G32B32A32_SINT:	  		return Coral::PixelFormat::RGBA32_I;
		case VK_FORMAT_R32G32B32_SINT:	  			return Coral::PixelFormat::RGB32_I;
		case VK_FORMAT_R32G32_SINT:		  			return Coral::PixelFormat::RG32_I;
		case VK_FORMAT_R32_SINT:			  		return Coral::PixelFormat::R32_I;

		case VK_FORMAT_R32G32B32A32_UINT:			return Coral::PixelFormat::RGBA32_UI;
		case VK_FORMAT_R32G32B32_UINT:				return Coral::PixelFormat::RGB32_UI;
		case VK_FORMAT_R32G32_UINT:					return Coral::PixelFormat::RG32_UI;
		case VK_FORMAT_R32_UINT:					return Coral::PixelFormat::R32_UI;

		case VK_FORMAT_R16G16B16A16_SINT:			return Coral::PixelFormat::RGBA16_I;
		case VK_FORMAT_R16G16B16_SINT:			 	return Coral::PixelFormat::RGB16_I;
		case VK_FORMAT_R16G16_SINT:				 	return Coral::PixelFormat::RG16_I;
		case VK_FORMAT_R16_SINT:					return Coral::PixelFormat::R16_I;

		case VK_FORMAT_R16G16B16A16_UINT:	 		return Coral::PixelFormat::RGBA16_UI;
		case VK_FORMAT_R16G16B16_UINT:	 			return Coral::PixelFormat::RGB16_UI;
		case VK_FORMAT_R16G16_UINT:		 			return Coral::PixelFormat::RG16_UI;
		case VK_FORMAT_R16_UINT:		 			return Coral::PixelFormat::R16_UI;

		case VK_FORMAT_R8G8B8A8_SINT:	   			return Coral::PixelFormat::RGBA8_I;
		case VK_FORMAT_R8G8B8_SINT:	   				return Coral::PixelFormat::RGB8_I;
		case VK_FORMAT_R8G8_SINT:		   			return Coral::PixelFormat::RG8_I;
		case VK_FORMAT_R8_SINT:		   				return Coral::PixelFormat::R8_I;

		case VK_FORMAT_R8G8B8A8_UINT:	  			return Coral::PixelFormat::RGBA8_UI;
		case VK_FORMAT_R8G8B8_UINT:	  				return Coral::PixelFormat::RGB8_UI;
		case VK_FORMAT_R8G8_UINT:		  			return Coral::PixelFormat::RG8_UI;
		case VK_FORMAT_R8_UINT:		  				return Coral::PixelFormat::R8_UI;

		case VK_FORMAT_R8G8B8A8_SRGB:				return Coral::PixelFormat::RGBA8_SRGB;
		case VK_FORMAT_R8G8B8_SRGB:					return Coral::PixelFormat::RGB8_SRGB;
		case VK_FORMAT_R8G8_SRGB:					return Coral::PixelFormat::RG8_SRGB;
		case VK_FORMAT_R8_SRGB:						return Coral::PixelFormat::R8_SRGB;

		default:
			std::unreachable();
	}
};


inline bool
isDepthFormat(PixelFormat format)
{
	switch (format)
	{
		case Coral::PixelFormat::DEPTH32_F:			
		case Coral::PixelFormat::DEPTH24_STENCIL8:	
		case Coral::PixelFormat::DEPTH16:			
			return true;
		default:
			return false;
	}
}


inline bool
isStencilFormat(PixelFormat format)
{
	switch (format)
	{
		case Coral::PixelFormat::DEPTH24_STENCIL8:
			return true;
		default:
			return false;
	}
}


inline VkBufferUsageFlags
convert(BufferType bufferType)
{
	switch (bufferType)
	{
		case Coral::BufferType::INDEX_BUFFER:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case Coral::BufferType::VERTEX_BUFFER:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case Coral::BufferType::STORAGE_BUFFER:
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case Coral::BufferType::UNIFORM_BUFFER:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	std::unreachable();	
}

} // namespace Coral::Vulkan

#endif // !CORAL_VULKAN_FORMAT_HPP