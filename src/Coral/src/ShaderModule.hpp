#ifndef CORAL_SHADERMODULE_HPP
#define CORAL_SHADERMODULE_HPP

#include <Coral/ShaderModule.h>

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <span>
#include <memory>
#include <unordered_set>

namespace Coral
{

/// Structure specifying a shader attribute binding
struct AttributeBindingInfo
{
    // The location in the shader of the attribute
    uint32_t location{ 0 };
    /// The format of the attribute
    CoAttributeFormat format{ CO_ATTRIBUTE_FORMAT_VEC3F };
    /// The name of the attribute in the shader
    std::string name;
};


using AttributeLayout = std::vector<AttributeBindingInfo>;

/// Definition of one member value of a uniform block
struct MemberDefinition
{
    /// The type of the value
    CoUniformFormat type;
    /// The name of the value
    std::string name;
    /// The number of elements in the member. A count > 1 defines an array with `count` values
    uint32_t count{ 1 };
    /// The offset in bytes from the start of the block to the first element of the member
    uint32_t offset{ 0 };
    /// The padded size in bytes
    uint32_t paddedSize{ 0 };
};

/// Defines a uniform block descriptor
struct UniformBlockDefinition
{
    std::vector<MemberDefinition> members;
};

using SamplerDefinition                = CoSamplerDefinition;
using TextureDefinition                = CoTextureDefinition;
using CombinedTextureSamplerDefinition = CoCombinedTextureSamplerDefinition;

/// Structure specifying a shader descriptor binding
struct DescriptorDefinition
{
    /// The binding index within the descriptor set to which a descriptor matching this definition must be bound
    uint32_t binding{ 0 };

    /// The name of the descriptor in the shader
    std::string name;

    /// The type definition of the descriptor
    std::variant<UniformBlockDefinition, SamplerDefinition, TextureDefinition, CombinedTextureSamplerDefinition> definition{ SamplerDefinition{} };
};


using DescriptorLayout = std::vector<DescriptorDefinition>;


class CORAL_API ShaderModule
{
public:

    /// Configuration to create a shader module object
    struct CreateConfig
    {
        /// The name of the shader
        std::string_view name;

        /// The stage of the shader
        CoShaderStage stage;

        /// The source code of the shader
        /**
         * \Note: Depending on the GraphicsAPI the shader source must be provided in the correct platform format:
         * * Vulkan: SpirV byte code in 4-byte words
         */
        std::span<const std::byte> source;

        /// The name of the entry-point function of the shader
        std::string_view entryPoint;
    };

    enum class CreateError
    {
        INTERNAL_ERROR
    };

    virtual ~ShaderModule() = default;

    /// Get the shader stage of the shader module
    virtual CoShaderStage shaderStage() const = 0;

    /// Get the name of the shader module
    virtual const std::string& name() const = 0;

    /// Get the entry point of the shader module
    virtual const std::string& entryPoint() const = 0;

    /// Get the layout of input attributes required by this shader
    virtual const AttributeLayout& inputAttributeLayout() const = 0;

    /// Get the layout of output attributes required by this shader
    virtual const AttributeLayout& outputAttributeLayout() const = 0;

    /// Get the layout of descriptors required by this shader
    virtual const DescriptorLayout& descriptorLayout() const = 0;
};

} // namespace Coral


struct CoShaderModule_T
{
    std::shared_ptr<Coral::ShaderModule> impl;

    mutable std::unordered_set<std::string> mStringCache;

    mutable std::vector<CoAttributeBindingInfo> mInputAttributeLayoutData;
    mutable std::vector<CoAttributeBindingInfo> mOutputAttributeLayoutData;

    mutable std::vector<CoMemberInfo> mMembersCache;
    mutable std::vector<CoDescriptorBindingInfo> mDescriptorBindingInfos;
    
};

#endif // !CORAL_SHADERMODULE_HPP
