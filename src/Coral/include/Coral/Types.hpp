#ifndef CORAL_TYPES_HPP
#define CORAL_TYPES_HPP

#include <Coral/System.hpp>

#include <cstdint>
#include <utility>

namespace Coral
{

///
struct Rectangle
{
	uint32_t x{ 0 };
	uint32_t y{ 0 };
	uint32_t width{ 0 };
	uint32_t height{ 0 };
};

/// Pixel format for textures
enum class PixelFormat
{
	R8_SRGB,
	RG8_SRGB,
	RGB8_SRGB,
	RGBA8_SRGB,

	R8_UI,
	RG8_UI,
	RGB8_UI,
	RGBA8_UI,

	R8_I,
	RG8_I,
	RGB8_I,
	RGBA8_I,

	R16_UI,
	RG16_UI,
	RGB16_UI,
	RGBA16_UI,

	R16_I,
	RG16_I,
	RGB16_I,
	RGBA16_I,

	R32_UI,
	RG32_UI,
	RGB32_UI,
	RGBA32_UI,

	R32_I,
	RG32_I,
	RGB32_I,
	RGBA32_I,

	R16_F,
	RG16_F,
	RGB16_F,
	RGBA16_F,

	R32_F,
	RG32_F,
	RGB32_F,
	RGBA32_F,

	DEPTH16,
	DEPTH24_STENCIL8,
	DEPTH32_F
};

/// Attribute format for buffers
enum class AttributeFormat
{
	UINT16,
	UINT32,
	INT16,
	INT32,
	FLOAT,
	VEC2F,
	VEC3F,
	VEC4F
};

/// Format of uniform members 
enum class UniformFormat
{
	BOOL,
	INT32,
	FLOAT,
	VEC2I,
	VEC3I,
	VEC4I,
	VEC2F,
	VEC3F,
	VEC4F,
	MAT33F,
	MAT44F
};

/// Get the size in bytes of the pixel format
constexpr inline uint32_t sizeInBytes(PixelFormat format)
{
	switch (format)
	{
		case PixelFormat::R8_UI:		return 1;
		case PixelFormat::R8_I:			return 1;
		case PixelFormat::R16_UI:		return 2;
		case PixelFormat::R16_I:		return 2;
		case PixelFormat::R32_UI:		return 4;
		case PixelFormat::R32_I:		return 4;
		case PixelFormat::R32_F:		return 4;

		case PixelFormat::RG8_UI:		return 2;
		case PixelFormat::RG8_I:		return 2;
		case PixelFormat::RG16_UI:		return 4;
		case PixelFormat::RG16_I:		return 4;
		case PixelFormat::RG32_UI:		return 8;
		case PixelFormat::RG32_I:		return 8;
		case PixelFormat::RG32_F:		return 8;

		case PixelFormat::RGB8_UI:		return 3;
		case PixelFormat::RGB8_I:		return 3;
		case PixelFormat::RGB16_UI:		return 6;
		case PixelFormat::RGB16_I:		return 6;
		case PixelFormat::RGB32_UI:		return 12;
		case PixelFormat::RGB32_I:		return 12;
		case PixelFormat::RGB32_F:		return 12;

		case PixelFormat::RGBA8_UI:		return 4;
		case PixelFormat::RGBA8_I:		return 4;
		case PixelFormat::RGBA16_UI:	return 8;
		case PixelFormat::RGBA16_I:		return 8;
		case PixelFormat::RGBA32_UI:	return 16;
		case PixelFormat::RGBA32_I:		return 16;
		case PixelFormat::RGBA32_F:		return 16;

		case PixelFormat::DEPTH16:			return 2;
		case PixelFormat::DEPTH24_STENCIL8:	return 4;
		case PixelFormat::DEPTH32_F:		return 4;

		case PixelFormat::R16_F:			return 2;
		case PixelFormat::RG16_F:			return 4;
		case PixelFormat::RGB16_F:			return 6;
		case PixelFormat::RGBA16_F:			return 8;

		case PixelFormat::R8_SRGB:			return 2;
		case PixelFormat::RG8_SRGB:			return 4;
		case PixelFormat::RGB8_SRGB:		return 6;
		case PixelFormat::RGBA8_SRGB:		return 8;
	}

	std::unreachable();
}

/// Get the size in bytes of the attribute format
constexpr inline uint32_t sizeInBytes(Coral::AttributeFormat format)
{
	switch (format)
	{
		case AttributeFormat::UINT16:	return 2;
		case AttributeFormat::UINT32:	return 4;
		case AttributeFormat::INT16:	return 2;
		case AttributeFormat::INT32:	return 4;
		case AttributeFormat::FLOAT:	return 4;
		case AttributeFormat::VEC2F:	return 8;
		case AttributeFormat::VEC3F:	return 12;
		case AttributeFormat::VEC4F:	return 16;
	}

	std::unreachable();
}

/// Get the size in bytes of the uniform format
constexpr inline size_t sizeInBytes(UniformFormat format)
{
	switch (format)
	{
		case UniformFormat::BOOL:	return 4;
		case UniformFormat::INT32:	return 4;
		case UniformFormat::FLOAT:	return 4;
		case UniformFormat::VEC2F:	return 8;
		case UniformFormat::VEC3F:	return 12;
		case UniformFormat::VEC4F:	return 16;
		case UniformFormat::VEC2I:	return 8;
		case UniformFormat::VEC3I:	return 12;
		case UniformFormat::VEC4I:	return 16;
		case UniformFormat::MAT33F:	return 36;
		case UniformFormat::MAT44F:	return 64;
	};

	std::unreachable();
}

} // namespace Coral

#endif // !CORAL_TYPES_HPP