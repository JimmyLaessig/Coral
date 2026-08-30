#ifndef CORAL_SHADERMODULE_H
#define CORAL_SHADERMODULE_H

#include <Coral/Context.h>

typedef enum
{
    CO_SHADER_STAGE_VERTEX   = 0,
    CO_SHADER_STAGE_FRAGMENT = 1,
} CoShaderStage;

/*!
 * Configuration to create a shader module object
 */
typedef struct
{
    /*!
     * The name of the shader
     */
    const char* pName;

    /*!
     * The stage of the shader
     */
    CoShaderStage stage;

    /*!
     * The source code of the shader
     * \Note: Depending on the GraphicsAPI the shader source must be provided in the correct platform format:
     * * Vulkan: SpirV byte code in 4-byte words
     */
    const CoByte* pSource;

    /*!
     * The number of bytes in the
     */
    uint32_t sourceCount;

    /*!
     * The name of the entry point function of the shader
     */
    const char* pEntryPoint;
} CoShaderModuleCreateConfig;


/*!
 * Structure specifying a shader attribute binding
 */
typedef struct 
{
    /*!
     * The location in the shader of the attribute
     */
    uint32_t location;

    /*!
     * The format of the attribute
     */
    CoAttributeFormat format;

    /*!
     * The name of the attribute in the shader
     */
    const char* pName;

} CoAttributeBindingInfo;

typedef enum
{
    CO_MATRIX_LAYOUT_ROW_MAJOR = 0,

    CO_MATRIX_LAYOUT_COLUMN_MAJOR,

} CoMatrixLayout;
/*!
 *
 */
typedef enum
{
    /*!
     * Describes a 32-bit boolean scalar
     */
    CO_SCALAR_TYPE_BOOL    = 0,
    /*!
     * Describes a 32-bit floating point scalar
     */
    CO_SCALAR_TYPE_FLOAT32,
    /*!
     * Describes a 64-bit floating point scalar
     */
    CO_SCALAR_TYPE_FLOAT64,
    /*!
     * Describes a 8-bit signed integer
     */
    CO_SCALAR_TYPE_INT8,
    /*!
     * Describes a 16-bit signed integer
     */
    CO_SCALAR_TYPE_INT16,
    /*!
     * Describes a 32-bit signed integer
     */
    CO_SCALAR_TYPE_INT32,
    /*!
     * Describes a 64-bit signed integer
     */
    CO_SCALAR_TYPE_INT64,
    /*!
     * Describes a 8-bit unsigned integer
     */
    CO_SCALAR_TYPE_UINT8,
    /*!
     * Describes a 16-bit unsigned integer
     */
    CO_SCALAR_TYPE_UINT16,
    /*!
     * Describes a 32-bit unsigned integer
     */
    CO_SCALAR_TYPE_UINT32,
    /*!
     * Describes a 64-bit unsigned integer
     */
    CO_SCALAR_TYPE_UINT64,

} CoScalarType;

/*!
 * Structure containing information about a scalar
 */
typedef struct
{
    /*!
     * Data type of the scalar
     */
    CoScalarType dataType;

} CoScalarInfo;

/*!
 * Structure containing information about a vector
 */
typedef struct
{
    /*!
     * Data type of the vector elements
     */
    CoScalarType dataType;

    /*!
     * Number of components in the vector
     */
    uint32_t componentCount;

} CoVectorInfo;

/*!
 * Structure containing information about a matrix
 */
typedef struct
{
    /*!
     * Data type of the matrix elements
     */
    CoScalarType dataType;

    /*!
     * Number of rows in the matrix
     */
    uint32_t rowCount;

    /*!
     * Number of columns in the matrix
     */
    uint32_t columnCount;

    /*!
     * The distance between the start of two consecutive rows/columnms the matrix in bytes. Only valid if count > 1
     */
    uint32_t stride;

    /*!
     * The memory layout of the matrix
     */
    CoMatrixLayout layout;

} CoMatrixInfo;

typedef struct
{
    /*!
     * Pointer to an array containing the dimensions of the array. The number of elements in the array is specified by `dimensionsCount`
     */
    const uint32_t* pDimensions;

    /*!
     * Number of elements in the dimensions array
     */
    uint32_t dimensionsCount;

} CoArrayInfo;

typedef struct CoStructMemberInfo CoStructMemberInfo;

/*!
 * Structure containg the information about a struct used in a uniform or storage buffer
 */
typedef struct
{
    /*!
     * The type name of the structure. Can be nullptr, if the structure has no name.
     */
    const char* pTypeName;

    /*!
     * Pointer to an array of CoStructMemberInfo structures
     */
    const CoStructMemberInfo* pMembers;

    /*!
     * Number of elements in the members array.
     */
    uint32_t memberCount;

} CoStructInfo;

typedef enum
{
    CO_STRUCT_MEMBER_TYPE_SCALAR,
    CO_STRUCT_MEMBER_TYPE_VECTOR,
    CO_STRUCT_MEMBER_TYPE_MATRIX,
    CO_STRUCT_MEMBER_TYPE_STRUCT,
} CoStructMemberType;

typedef enum
{
    CO_STRUCT_MEMBER_TYPE_FLAG_NONE_BIT    = 0x0,
    CO_STRUCT_MEMBER_TYPE_FLAG_POINTER_BIT = 0x1
} CoStructMemberTypeFlagBits;

typedef uint32_t CoStructMemberTypeFlags;

/*!
 * Structure describing a member variable of a struct used in a uniform or storage buffer
 */
struct CoStructMemberInfo
{
    /*!
     * The name of the value
     */
    const char* pName;

    /*!
     * The number of elements in the member. A count > 1 defines an array with `count` values
     */
    uint32_t count;

    /*!
     * The offset of the member in bytes from the start of the struct.
     */
    uint32_t offset;

    /*!
     * Distance between the start of two consecutive elements of the member in bytes. Only valid if count > 1
     */
    uint32_t stride;

    /*!
     * The type of the member
     */
    CoStructMemberType type;

    union
    {
        /*!
         * The scalar info of the member. Only valid if type is CO_STRUCT_MEMBER_TYPE_SCALAR
         */
        CoScalarInfo scalar;

        /*!
         * The vector info of the member. Only valid if type is CO_STRUCT_MEMBER_TYPE_VECTOR
         */
        CoVectorInfo vector;

        /*!
         * The matrix info of the member. Only valid if type is CO_STRUCT_MEMBER_TYPE_MATRIX
         */
        CoMatrixInfo matrix;

        /*!
         * The structure info of the member. Only valid if type is CO_STRUCT_MEMBER_TYPE_STRUCT
         */
        CoStructInfo structure;
    };

    CoStructMemberTypeFlags typeFlags;
};

/*!
 * Description of a storage buffer
 */
typedef struct
{
    /*!
     * Structure info of the buffer
     */
    CoStructMemberInfo structure;

} CoBufferInfo;

/*!
 * Structure specifying a uniform sampler descriptor
 */
typedef struct 
{
} CoSamplerInfo;

/*!
 * Defines a uniform image descriptor
 */
typedef struct 
{
} CoImageInfo;

/*!
 * Defines a uniform texture descriptor
 */
typedef struct 
{
} CoCombinedImageSamplerInfo;

/*!
 * Structure specifying a shader descriptor binding
 */
typedef struct 
{
    /*!
     * The binding index to which a descriptor matching this definition must be bound
     */
    uint32_t binding;

    /*!
     * The name of the descriptor
     */
    const char* pName;

    /*!
     * The type of the descriptor
     */
    CoDescriptorType type;

    union
    {
        CoBufferInfo buffer;
        CoImageInfo image;
        CoSamplerInfo sampler;
        CoCombinedImageSamplerInfo combinedImageSampler;
    };

} CoDescriptorInfo;

typedef struct
{
    /*!
     * Pointer to an array of CoDescriptorInfo structures that describe the descriptors used by the shader module
     */
    const CoDescriptorInfo* pDescriptorInfos;

    /*!
     * Number of elements in the descriptorInfos array
     */
    uint32_t descriptorInfosCount;
} CoDescriptorLayout;

typedef struct
{
    /*!
     * Pointer to an array of CoAttributeBindingInfo structurres that describe the input attributes of the shader module
     */
    const CoAttributeBindingInfo* pInputAttributeBindingInfos;

    /*!
     * Number of elements in the inputAttributeBindingInfos array
     */
    uint32_t inputAttributeBindingInfoCount;

    /*!
     * Pointer to an array of CoAttributeBindingInfo structures that describe the output attributes of the shader module
     */
    const CoAttributeBindingInfo* pOutputAttributeBindingInfos;

    /*!
     * Number of elements in the outputAttributeBindingInfos array
     */
    uint32_t outputAttributeBindingInfoCount;

} CoAttributeLayout;

struct CoShaderModule_T;

typedef CoShaderModule_T* CoShaderModule;

CORAL_API CoResult coContextCreateShaderModule(CoContext context, 
                                               const CoShaderModuleCreateConfig* pConfig, 
                                               CoShaderModule* pShaderModule);

CORAL_API void coDestroyShaderModule(CoShaderModule shaderModule);


/*!
 * \brief Get the descriptor layout of the shader module
 * \param shaderModule The shader module to get the descriptor layout from
 * \param pLayout Pointer to a CoDescriptorLayout structure that will be filled with the descriptor layout of the shader module
 */
CORAL_API void coShaderModuleGetDescriptorLayout(const CoShaderModule shaderModule, CoDescriptorLayout* pLayout);

/*!
 * \brief Get the descriptor layout of the shader module
 * \param shaderModule The shader module to get the descriptor layout from
 * \param pLayout Pointer to a CoDescriptorLayout structure that will be filled with the descriptor layout of the shader module
 */
CORAL_API void coShaderModuleGetAttributeLayout(const CoShaderModule shaderModule, CoDescriptorLayout* pLayout);

#endif // !CORAL_SHADERMODULE_H
