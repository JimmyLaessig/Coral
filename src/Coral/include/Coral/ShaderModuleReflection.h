#ifndef CORAL_SHADERMODULEREFLECTION_H
#define CORAL_SHADERMODULEREFLECTION_H

#include <Coral/ShaderModule.h>

/*!
 * Enum describing the data type of a scalar
 */
typedef enum
{
    /*!
     * Describes a 32-bit boolean scalar
     */
    CO_SCALAR_TYPE_BOOL = 0,
    /*!
    * Describes a 16-bit floating point scalar
    */
    CO_SCALAR_TYPE_FLOAT16,
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
 * \brief Get the size in bytes of a scalar type
 * \return The size in bytes of the scalar type
 */
CORAL_API uint32_t coScalarTypeGetSizeInBytes(const CoScalarType scalarType);

/*!
 * Structure describing a scalar
 */
typedef struct
{
    /*!
     * Data type of the scalar
     */
    CoScalarType dataType;

    /*!
     * Size of the scalar in bytes
     */
    uint32_t size;

} CoScalarInfo;

/*!
 * Structure describing a vector
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
    uint32_t components;

    /*!
     * Size of the vector in bytes
     */
    uint32_t size;

} CoVectorInfo;

/*!
 * Memory layout of matrices
 */
typedef enum
{
    /*!
     * Matrix elements are stored row by row in contiguous memory, starting
     * with the first row, followed by the second row, and so on.
     */
    CO_MATRIX_LAYOUT_ROW_MAJOR = 0,

    /*!
     * Matrix elements are stored column by column in contiguous memory,
     * starting with the first column, followed by the second column, and so
     * on.
     */
    CO_MATRIX_LAYOUT_COLUMN_MAJOR,

} CoMatrixLayout;

/*!
 * Structure describing a matrix
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
    uint32_t rows;

    /*!
     * Number of columns in the matrix
     */
    uint32_t columns;

    /*!
     * The distance between the start of two consecutive rows/columns the matrix in bytes.
     */
    uint32_t stride;

    /*!
     * The memory layout of the matrix
     */
    CoMatrixLayout layout;

} CoMatrixInfo;

/*!
 * Structure describing an array
 */
typedef struct
{
    /*!
     * Pointer to an array containing the dimensions of the array. The number of elements in the array is specified by `dimensionCount`
     */
    const uint32_t* pDimensions;

    /*!
     * Number of elements in the dimensions array
     */
    uint32_t dimensionCount;

    /*!
     * Distance between the start of two consecutive elements in the array in bytes
     */
    uint32_t stride;

} CoArrayInfo;

/*!
 * Structure describing a member variable of a structure used in a uniform or storage buffer
 */
typedef struct CoStructureMemberInfo CoStructureMember;

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
    const CoStructureMemberInfo* pMembers;

    /*!
     * Number of elements in the members array.
     */
    uint32_t memberCount;

    /*!
     * Size of the structure in bytes
     */
    uint32_t size;

} CoStructureInfo;


typedef enum
{
    /*!
     * The member is a scalar
     */
    CO_STRUCTURE_MEMBER_TYPE_SCALAR,
    /*!
     * The member is a vector
     */
    CO_STRUCTURE_MEMBER_TYPE_VECTOR,
    /*!
     * The member is a matrix
     */
    CO_STRUCTURE_MEMBER_TYPE_MATRIX,
    /*!
     * The member is a structure
     */
    CO_STRUCTURE_MEMBER_TYPE_STRUCTURE,

} CoStructureMemberType;

/*!
 * Decoration flags of structure members
 */
typedef enum
{
    CO_STRUCTURE_MEMBER_TYPE_FLAG_NONE_BIT    = 0x0,

    /*!
     * The member is a pointer
     */
    CO_STRUCTURE_MEMBER_TYPE_FLAG_POINTER_BIT = 0x1
} CoStructureMemberTypeFlagBits;

/*!
 * Bit set of decoration flags of structure members. See \ref CoStructureMemberTypeFlagBits
 * for all allowed bit flags
 */
typedef uint32_t CoStructureMemberTypeFlags;

/*!
 * Structure describing a member variable of a structure used in a uniform or storage buffer
 */
struct CoStructureMemberInfo
{
    /*!
     * Null-terminated string containing the name of the value
     */
    const char* pName;

    /*!
     * The total offset from the start of the structure outermost structure to the start of the
     * member in bytes.
     */
    uint32_t offset;

    /*!
     * The type of the member
     */
    CoStructureMemberType type;

    /*!
     * The array info of the member. If the member is not an array, dimensionCount is 0 and
     * pDimensions is must be nullptr
     */
    CoArrayInfo array;

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
        CoStructureInfo structure;
    };

    /*!
     * Decoration flags for the member. See \ref CoStructureMemberTypeFlagBits for all allowed
     * bit flags
     */
    CoStructureMemberTypeFlags typeFlags;
};

/*!
 * Structure specifying a buffer descriptor
 */
typedef struct
{
    /*!
     * Structure info of the buffer
     */
    CoStructureInfo structure;

    /*!
     * Array info of the buffer
     */
    CoArrayInfo array;

} CoBufferInfo;

/*!
 * Structure specifying a sampler descriptor
 */
typedef struct 
{
} CoSamplerInfo;

/*!
 * Structure specifying an image descriptor
 */
typedef struct 
{
} CoImageInfo;

/*!
 * Structure specifying a combined image and sampler descriptor
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

} CoReflectDescriptor;

/*!
 * Enum describing the type of an attribute
 */
typedef enum
{
    /*!
     * The attribute is a scalar 
     */
    CO_ATTRIBUTE_TYPE_SCALAR,
    /*!
     * The attribute is a vector
     */
    CO_ATTRIBUTE_TYPE_VECTOR,
    /*!
     * The attribute is a matrix
     */
    CO_ATTRIBUTE_TYPE_MATRIX,

} CoAttributeType;

/*!
 * Reflection information of a shader attribute
 */
typedef struct
{
    /*!
     * Null-terminated string containing the name of the attribute
     */
    const char* pName;

    /*!
     * The location of the attribute
     */
    uint32_t location;

    /*!
     * The format of the attribute
     */
    CoAttributeFormat format;

} CoReflectAttribute;

/*!
 * Reflection information of a descriptor set
 */
typedef struct
{
    /*!
     * Pointer to an array of CoReflectDescriptor structures that describe the descriptors used by the shader module
     */
    const CoReflectDescriptor* pDescriptors;

    /*!
     * Number of elements in the descriptors array
     */
    uint32_t descriptorCount;

    /*!
     * The index of the Descriptor Set
     */
    uint32_t set;

} CoReflectDescriptorSet;

/*!
 * Reflection information of a shader module
 */
typedef struct
{
    /*!
     * Pointer to an array of CoReflectAttribute structures that describe
     * the input attributes of the shader module
     */
    const CoReflectAttribute* pInputAttributes;

    /*!
     * Number of elements in the inputAttributes array
     */
    uint32_t inputAttributeCount;

    /*!
     * Pointer to an array of CoReflectAttribute structures that describe
     * the output attributes of the shader module
     */
    const CoReflectAttribute* pOutputAttributes;

    /*!
     * Number of elements in the inputAttributes array
     */
    uint32_t outputAttributeCount;

    /*!
     * Pointer to an array of CoReflectDescriptorSet structures that describe
     * the descriptor sets of the shader module
     */
    const CoReflectDescriptorSet* pDescriptorSets;

    /*!
     * Number of elements in the descriptorSets array
     */
    uint32_t descriptorSetCount;

} CoShaderModuleReflectionInfo;

/*!
 * \brief Reflect the properties of the given shader module
 * The returned reflection information is owned by the shader module and is valid until the shader module is destroyed.
 * \param shaderModule The shader module to get the descriptor layout from
 * \param pShaderModuleReflectionInfo Pointer to a CoShaderModuleReflectionInfo structure that will be filled
 *        with the reflection information of the shader module
 */
CORAL_API void coShaderModuleReflect(const CoShaderModule shaderModule, CoShaderModuleReflectionInfo* pShaderModuleReflectionInfo);

#endif // !CORAL_SHADERMODULEREFLECTION_H
