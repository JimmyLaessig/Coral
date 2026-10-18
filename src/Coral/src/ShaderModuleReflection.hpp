#ifndef CORAL_SHADERMODULEREFLECTION_HPP
#define CORAL_SHADERMODULEREFLECTION_HPP

#include <Coral/ShaderModuleReflection.h>

#include <variant>
#include <vector>
#include <string>

namespace Coral
{

/*!
 * Structure describing a scalar
 */
struct ScalarInfo
{
    /*!
     * Data type of the scalar
     */
    CoScalarType dataType;

}; // struct ScalarInfo

/*!
 * Structure describing a vector
 */
struct VectorInfo
{
    /*!
     * Data type of the vector elements
     */
    CoScalarType dataType;

    /*!
     * Number of components in the vector
     */
    uint32_t components;

}; // struct VectorInfo

/*!
 * Structure describing a matrix
 */
struct MatrixInfo
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

}; // struct MatrixInfo

/*!
 * Structure describing an array
 */
struct ArrayInfo
{
   /*!
    * Vector containing the dimensions of the array
    */
   std::vector<uint32_t> dimensions;

   /*!
    * Distance between the start of two consecutive elements in the array in bytes
    */
   uint32_t stride;

}; // struct ArrayInfo

/*!
 * Structure describing a member variable of a structure used in a uniform or storage buffer
 */
struct StructureMember;

/*!
 * Structure containg the information about a struct used in a uniform or storage buffer
 */
struct StructureInfo
{
    /*!
     * The type name of the structure. Can be empty, if the structure has no name.
     */
    std::string typeName;

    /*!
     * The members of the structure
     */
    std::vector<StructureMember> members;

    /*!
     * Size of the structure in bytes
     */
    uint32_t size;

}; // struct StructureInfo

/*!
 * Structure describing a member variable of a structure used in a uniform or storage buffer
 */
struct StructureMember
{
    /*!
     * The name of the value
     */
    std::string name;

    /*!
     * The offset of the member in bytes from the start of the structure.
     */
    uint32_t offset{ 0 };

    /*!
     * Array info of the member
     */
    ArrayInfo array;

    /*!
     * Type info of the member
     */
    std::variant<ScalarInfo, VectorInfo, MatrixInfo, StructureInfo> typeInfo;

    /*!
     * Decoration flags of the member
     */
    CoStructureMemberTypeFlags typeFlags;

}; // struct StructureMember

/*!
 * Structure specifying a buffer descriptor
 */
struct BufferInfo
{
    /*!
     * The type of the buffer
     */
    CoBufferType bufferType;

    /*!
     * Structure info of the buffer
     */
    Coral::StructureInfo structure;

}; // struct BufferInfo

/*!
 * Structure specifying a sampler descriptor
 */
struct SamplerInfo
{
}; // struct SamplerInfo

/*!
 * Structure specifying an image descriptor
 */
struct ImageInfo
{
}; // struct ImageInfo

/*!
 * Structure specifying a combined image and sampler descriptor
 */
struct CombinedImageSamplerInfo
{
}; // struct CombinedImageSamplerInfo

using DescriptorInfo = std::variant<BufferInfo, ImageInfo, SamplerInfo, CombinedImageSamplerInfo>;

/*!
 * Structure specifying a shader descriptor binding
 */
struct ReflectDescriptor
{
    /*!
     * The binding index to which a descriptor matching this definition must be bound
     */
    uint32_t binding{ 0 };

    /*!
     * The name of the descriptor
     */
    std::string name;

    /*!
     * The info of the descriptor
     */
    DescriptorInfo info;

}; // struct ReflectDescriptor

/*!
 * Reflection information of a descriptor set
 */
struct ReflectDescriptorSet
{
    /*!
     * The descriptors in the descriptor set
     */
    std::vector<ReflectDescriptor> descriptors;

    /*!
     * The index of the descriptor set
     */
    uint32_t set{ 0 };

}; // struct ReflectDescriptorSet

using AttributeInfo = std::variant<ScalarInfo, VectorInfo, MatrixInfo>;

/*!
 * Reflection information of a shader attribute
 */
struct ReflectAttribute
{
    /*!
     * The location of the attribute
     */
    uint32_t location{ 0 };

    /*!
     * The name of the attribute
     */
    std::string name;

    /*!
     * The format of the attribute
     */
    CoAttributeFormat format{ CO_ATTRIBUTE_FORMAT_RGBA_F32 };

}; // struct ReflectAttribute

/*!
 * Reflection information of a shader module
 */
struct ShaderModuleReflectionInfo
{
    /*!
     * The input attributes of the shader module
     */
    std::vector<ReflectAttribute> inputAttributes;

    /*!
     * The output attributes of the shader module
     */
    std::vector<ReflectAttribute> outputAttributes;

    /*!
     * The descriptor sets of the shader module
     */
    std::vector<ReflectDescriptorSet> descriptorSets;

}; // struct ReflectShaderModule

/*!
 * Create a CoReflectShaderModule object
 */
CoShaderModuleReflectionInfo create(const ShaderModuleReflectionInfo& shaderModule);

/*!
 * Destroy the CoReflectShaderModule object 
 */
void destroy(const CoShaderModuleReflectionInfo& shaderModule);

} // namespace Coral

#endif // !CORAL_SHADERMODULEREFLECTION_HPP
