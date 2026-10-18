#ifndef CORAL_TYPES_H
#define CORAL_TYPES_H

#include <Coral/Export.h>

#include <cstdint>

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
    // A timeout occurred
    CO_ERROR_TIMEOUT,
    // An invalid argument was passed to a function
    CO_ERROR_INVALID_ARGUMENT,

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

/*!
 * \brief Enum specifying the format of pixels in an image
 */
typedef enum
{
    CO_PIXEL_FORMAT_R8_SRGB = 0,
    CO_PIXEL_FORMAT_RG8_SRGB,
    CO_PIXEL_FORMAT_RGB8_SRGB,
    CO_PIXEL_FORMAT_RGBA8_SRGB,

    CO_PIXEL_FORMAT_R_U8,
    CO_PIXEL_FORMAT_RG_U8,
    CO_PIXEL_FORMAT_RGB_U8,
    CO_PIXEL_FORMAT_RGBA_U8,

    CO_PIXEL_FORMAT_R_I8,
    CO_PIXEL_FORMAT_RG_I8,
    CO_PIXEL_FORMAT_RGB_I8,
    CO_PIXEL_FORMAT_RGBA_I8,

    CO_PIXEL_FORMAT_R_U16,
    CO_PIXEL_FORMAT_RG_U16,
    CO_PIXEL_FORMAT_RGB_U16,
    CO_PIXEL_FORMAT_RGBA_U16,

    CO_PIXEL_FORMAT_R_I16,
    CO_PIXEL_FORMAT_RG_I16,
    CO_PIXEL_FORMAT_RGB_I16,
    CO_PIXEL_FORMAT_RGBA_I16,

    CO_PIXEL_FORMAT_R_U32,
    CO_PIXEL_FORMAT_RG_U32,
    CO_PIXEL_FORMAT_RGB_U32,
    CO_PIXEL_FORMAT_RGBA_U32,

    CO_PIXEL_FORMAT_R_I32,
    CO_PIXEL_FORMAT_RG_I32,
    CO_PIXEL_FORMAT_RGB_I32,
    CO_PIXEL_FORMAT_RGBA_I32,

    CO_PIXEL_FORMAT_R_F16,
    CO_PIXEL_FORMAT_RG_F16,
    CO_PIXEL_FORMAT_RGB_F16,
    CO_PIXEL_FORMAT_RGBA_F16,

    CO_PIXEL_FORMAT_R_F32,
    CO_PIXEL_FORMAT_RG_F32,
    CO_PIXEL_FORMAT_RGB_F32,
    CO_PIXEL_FORMAT_RGBA_F32,

    CO_PIXEL_FORMAT_DEPTH16,
    CO_PIXEL_FORMAT_DEPTH24_STENCIL8,
    CO_PIXEL_FORMAT_DEPTH_F32,

} CoPixelFormat;

/*!
 * \brief Enum specifying the format of an index
 */
typedef enum
{
    CO_INDEX_FORMAT_UINT16 = 0,
    CO_INDEX_FORMAT_UINT32,
} CoIndexFormat;

/*!
 * \brief Enum specifying the format of an attribute
 */
typedef enum
{
    CO_ATTRIBUTE_FORMAT_R_U16 = 0,
    CO_ATTRIBUTE_FORMAT_R_I16,
    CO_ATTRIBUTE_FORMAT_R_F16,
    CO_ATTRIBUTE_FORMAT_RG_U16,
    CO_ATTRIBUTE_FORMAT_RG_I16,
    CO_ATTRIBUTE_FORMAT_RG_F16,
    CO_ATTRIBUTE_FORMAT_RGB_U16,
    CO_ATTRIBUTE_FORMAT_RGB_I16,
    CO_ATTRIBUTE_FORMAT_RGB_F16,
    CO_ATTRIBUTE_FORMAT_RGBA_U16,
    CO_ATTRIBUTE_FORMAT_RGBA_I16,
    CO_ATTRIBUTE_FORMAT_RGBA_F16,
    CO_ATTRIBUTE_FORMAT_R_U32,
    CO_ATTRIBUTE_FORMAT_R_I32,
    CO_ATTRIBUTE_FORMAT_R_F32,
    CO_ATTRIBUTE_FORMAT_RG_U32,
    CO_ATTRIBUTE_FORMAT_RG_I32,
    CO_ATTRIBUTE_FORMAT_RG_F32,
    CO_ATTRIBUTE_FORMAT_RGB_U32,
    CO_ATTRIBUTE_FORMAT_RGB_I32,
    CO_ATTRIBUTE_FORMAT_RGB_F32,
    CO_ATTRIBUTE_FORMAT_RGBA_U32,
    CO_ATTRIBUTE_FORMAT_RGBA_I32,
    CO_ATTRIBUTE_FORMAT_RGBA_F32,
    CO_ATTRIBUTE_FORMAT_R_U64,
    CO_ATTRIBUTE_FORMAT_R_I64,
    CO_ATTRIBUTE_FORMAT_R_F64,
    CO_ATTRIBUTE_FORMAT_RG_U64,
    CO_ATTRIBUTE_FORMAT_RG_I64,
    CO_ATTRIBUTE_FORMAT_RG_F64,
    CO_ATTRIBUTE_FORMAT_RGB_U64,
    CO_ATTRIBUTE_FORMAT_RGB_I64,
    CO_ATTRIBUTE_FORMAT_RGB_F64,
    CO_ATTRIBUTE_FORMAT_RGBA_U64,
    CO_ATTRIBUTE_FORMAT_RGBA_I64,
    CO_ATTRIBUTE_FORMAT_RGBA_F64,

} CoAttributeFormat;

/*! 
 * \brief Get the size of the pixel format in bytes
 * \param format The pixel format
 * \return The size in bytes
 */
CORAL_API uint32_t coPixelFormatGetSizeInBytes(CoPixelFormat format);

/*!
 * \brief Get the size of the attribute format in bytes
 * \param format The attribute format
 * \return The size in bytes
 */
CORAL_API uint32_t coAttributeFormatGetSizeInBytes(CoAttributeFormat format);

/*!
 * \brief Get the size of the index format in bytes
 * \param format The index format
 * \return The size in bytes
 */
CORAL_API uint32_t coIndexFormatGetSizeInBytes(CoIndexFormat format);

typedef enum
{
    CO_CLEAR_OP_CLEAR = 0,
    CO_CLEAR_OP_LOAD,
    CO_CLEAR_OP_DONT_CARE,
} CoClearOp;

/*!
 * Enum specifying the type of a buffer
 */
typedef enum
{
    /*!
     * The buffer is used as a vertex buffer
     */
    CO_BUFFER_TYPE_VERTEX = 0,

    /*!
     * The buffer is used as an index buffer
     */
    CO_BUFFER_TYPE_INDEX,
    /*!
     * The buffer is used as uniform buffer
     */
    CO_BUFFER_TYPE_UNIFORM,
    /*!
     * The buffer is used as storage buffer
     */
    CO_BUFFER_TYPE_STORAGE,
    /*!
     * The buffer is used as indirect buffer
     */
    CO_BUFFER_TYPE_INDIRECT,
} CoBufferType;

/*!
 * Enum specifying the type of a descriptor
 */
typedef enum
{
    CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 0,
    CO_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    CO_DESCRIPTOR_TYPE_IMAGE,
    CO_DESCRIPTOR_TYPE_SAMPLER,
    CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
} CoDescriptorType;

#endif // !CORAL_TYPES_H
