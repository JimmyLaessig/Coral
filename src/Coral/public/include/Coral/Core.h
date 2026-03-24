#ifndef CORAL_TYPES_H
#define CORAL_TYPES_H

#include <Coral/System.hpp>

#include <cstdint>
#include <utility>

typedef enum
{
	// The operation was successful
	CO_SUCCESS = 0,
	// The operation failed
	CO_FAILED,
	// The selected Graphics API is not supported on this platform
	CO_ERROR_UNSUPPORTED_GRAPHICS_API,
	// The device ran out of memory
	CO_ERROR_OUT_OF_MEMORY,
	// The chosen size is invalid
	CO_ERROR_INVALID_SIZE,
	// One or more color attachments are invalid
	CO_ERROR_INVALID_COLOR_ATTACHMENT,
	// The depth-stencil attachment is invalid
	CO_ERROR_INVALID_DEPTH_STENCIL_ATTACHMENT,
	// An internal error occurred
	CO_ERROR_INTERNAL,

} CoResult;

typedef uint8_t CoByte;

typedef struct
{
	uint32_t width;
	uint32_t height;
} CoExtent;

typedef struct
{
	int32_t bottom;
	int32_t left;
	CoExtent extent;
} CoRectangle;

typedef enum
{
	CO_PIXEL_FORMAT_R8_SRGB,
	CO_PIXEL_FORMAT_RG8_SRGB,
	CO_PIXEL_FORMAT_RGB8_SRGB,
	CO_PIXEL_FORMAT_RGBA8_SRGB,

	CO_PIXEL_FORMAT_R8_UI,
	CO_PIXEL_FORMAT_RG8_UI,
	CO_PIXEL_FORMAT_RGB8_UI,
	CO_PIXEL_FORMAT_RGBA8_UI,

	CO_PIXEL_FORMAT_R8_I,
	CO_PIXEL_FORMAT_RG8_I,
	CO_PIXEL_FORMAT_RGB8_I,
	CO_PIXEL_FORMAT_RGBA8_I,

	CO_PIXEL_FORMAT_R16_UI,
	CO_PIXEL_FORMAT_RG16_UI,
	CO_PIXEL_FORMAT_RGB16_UI,
	CO_PIXEL_FORMAT_RGBA16_UI,

	CO_PIXEL_FORMAT_R16_I,
	CO_PIXEL_FORMAT_RG16_I,
	CO_PIXEL_FORMAT_RGB16_I,
	CO_PIXEL_FORMAT_RGBA16_I,

	CO_PIXEL_FORMAT_R32_UI,
	CO_PIXEL_FORMAT_RG32_UI,
	CO_PIXEL_FORMAT_RGB32_UI,
	CO_PIXEL_FORMAT_RGBA32_UI,

	CO_PIXEL_FORMAT_R32_I,
	CO_PIXEL_FORMAT_RG32_I,
	CO_PIXEL_FORMAT_RGB32_I,
	CO_PIXEL_FORMAT_RGBA32_I,

	CO_PIXEL_FORMAT_R16_F,
	CO_PIXEL_FORMAT_RG16_F,
	CO_PIXEL_FORMAT_RGB16_F,
	CO_PIXEL_FORMAT_RGBA16_F,

	CO_PIXEL_FORMAT_R32_F,
	CO_PIXEL_FORMAT_RG32_F,
	CO_PIXEL_FORMAT_RGB32_F,
	CO_PIXEL_FORMAT_RGBA32_F,

	CO_PIXEL_FORMAT_DEPTH16,
	CO_PIXEL_FORMAT_DEPTH24_STENCIL8,
	CO_PIXEL_FORMAT_DEPTH32_F,

} CoPixelFormat;


typedef enum
{
	CO_INDEX_FORMAT_UINT16 = 0,
	CO_INDEX_FORMAT_UINT32 = 1,
} CoIndexFormat;

typedef enum
{
	CO_ATTRIBUTE_FORMAT_UINT16 = 0,
	CO_ATTRIBUTE_FORMAT_UINT32 = 1,
	CO_ATTRIBUTE_FORMAT_INT16  = 2,
	CO_ATTRIBUTE_FORMAT_INT32  = 3,
	CO_ATTRIBUTE_FORMAT_FLOAT  = 4,
	CO_ATTRIBUTE_FORMAT_VEC2F  = 5,
	CO_ATTRIBUTE_FORMAT_VEC3F  = 6,
	CO_ATTRIBUTE_FORMAT_VEC4F  = 7
} CoAttributeFormat;

typedef enum
{
	CO_UNIFORM_FORMAT_BOOL   = 0,
	CO_UNIFORM_FORMAT_INT32  = 1,
	CO_UNIFORM_FORMAT_FLOAT  = 2,
	CO_UNIFORM_FORMAT_VEC2I  = 3,
	CO_UNIFORM_FORMAT_VEC3I  = 4,
	CO_UNIFORM_FORMAT_VEC4I  = 5,
	CO_UNIFORM_FORMAT_VEC2F  = 6,
	CO_UNIFORM_FORMAT_VEC3F  = 7,
	CO_UNIFORM_FORMAT_VEC4F  = 8,
	CO_UNIFORM_FORMAT_MAT33F = 9,
	CO_UNIFORM_FORMAT_MAT44F = 10,
} CoUniformFormat;


/// Get the size in bytes of the pixel format
uint32_t coPixelFormatGetSizeInBytes(CoPixelFormat format);

/// Get the size in bytes of the attribute format
uint32_t coAttributeFormatGetSizeInBytes(CoAttributeFormat format);

/// Get the size in bytes of the uniform format
/**
 * Returns the packed size of the uniform format without padding or alignment.
 */
uint32_t coUniformFormatGetSizeInBytes(CoUniformFormat format);

/// Get the size in bytes of the index format
uint32_t coIndexFormatGetSizeInBytes(CoIndexFormat format);

typedef enum
{
	CO_CLEAR_OP_CLEAR     = 0,
	CO_CLEAR_OP_LOAD      = 1,
	CO_CLEAR_OP_DONT_CARE = 2,
} CoClearOp;

#endif // !CORAL_TYPES_H
