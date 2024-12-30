#ifndef CORAL_TYPES_HPP
#define CORAL_TYPES_HPP

#include <Coral/System.hpp>

#include <cstdint>

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


/// Attribute format for buffer elements
enum class AttributeFormat
{
	UINT16,
	UINT32,
	INT16,
	INT32,
	FLOAT,
	VEC2,
	VEC3,
	VEC4
};


/// Get the size in bytes for the pixel format
constexpr inline uint32_t sizeInBytes(PixelFormat format)
{
	using namespace Coral;
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
	}

	return 0;
}


/// Get the size in bytes for the attribute format
constexpr inline uint32_t sizeInBytes(Coral::AttributeFormat type)
{
	switch (type)
	{
		case AttributeFormat::UINT16:	return 2;
		case AttributeFormat::UINT32:	return 4;
		case AttributeFormat::INT16:	return 2;
		case AttributeFormat::INT32:	return 4;
		case AttributeFormat::FLOAT:	return 4;
		case AttributeFormat::VEC2:		return 8;
		case AttributeFormat::VEC3:		return 12;
		case AttributeFormat::VEC4:		return 16;
	}

	return 0;
}

} // namespace Coral

#endif // !CORAL_TYPES_HPP