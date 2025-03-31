#ifndef CORAL_SHADERMODULE_HPP
#define CORAL_SHADERMODULE_HPP

#include <Coral/System.hpp>
#include <Coral/Types.hpp>

#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <span>

namespace Coral
{

/// The stage for which the shader module is used
enum class ShaderStage
{
	/// The shader module is used for the vertex shader stage
	VERTEX,
	/// The shader module is used for the fragment shader stage
	FRAGMENT
};

/// Configuration to create a shader module object
struct ShaderModuleCreateConfig
{
	/// The name of the shader
	std::string_view name;

	/// The stage of the shader
	ShaderStage stage;

	/// The source code of the shader
	/**
	 * \Note: Depending on the GraphicsAPI the shader source must be provided in the correct platform format:
	 * * Vulkan: SpirV byte code in 4-byte words
	 */
	std::span<const std::byte> source;

	/// The name of the entry-point function of the shader
	std::string_view entryPoint;
};


enum class ShaderModuleCreationError
{
	INTERNAL_ERROR
};

/// Definition of a shader attribute of the shader module
struct AttributeBindingLayout
{
	/// The buffer binding index
	uint32_t binding{ 0 };
	// The location in the shader of the attribute
	uint32_t location{ 0 };
	/// The format of the attribute
	Coral::AttributeFormat format{ Coral::AttributeFormat::VEC3F };
	/// The name of the attribute in the shader
	std::string name;
};

/// Description of one member value of a uniform block
struct MemberDefinition
{
	/// The type of the value
	UniformFormat type;
	/// The name of the value
	std::string name;
	/// The number of elements in the member. A count > 1 defines an array with `count` values
	uint32_t count{ 1 };
	/// The size in bytes of the member
	uint32_t size{ 0 };
	/// The padded size in bytes
	uint32_t paddedSize{ 0 };
};

/// Defines a uniform block descriptor
struct UniformBlockDefinition
{
	std::vector<MemberDefinition> members;
};

/// Defines a uniform sampler descriptor
struct SamplerDefinition
{
};

/// Defines a uniform texture descriptor
struct TextureDefinition
{
};

/// Defines a uniform texture descriptor
struct CombinedTextureSamplerDefinition
{
};


using DescriptorDefinition = std::variant<UniformBlockDefinition, SamplerDefinition, TextureDefinition, CombinedTextureSamplerDefinition>;

/// Definition of a descriptor required by the shader
struct DescriptorBindingLayout
{
	/// The binding index within the descriptor set to which a descriptor matching this definition must be bound
	uint32_t binding{ 0 };

	/// The name of the descriptor in the shader
	std::string name;

	/// The size in bytes of the descriptor
	size_t byteSize{ 0 };

	/// The type definition of the descriptor
	DescriptorDefinition definition{ SamplerDefinition{} };
};


class CORAL_API ShaderModule
{
public:

	virtual ~ShaderModule() = default;

	/// Get the shader stage of the shader module
	virtual ShaderStage shaderStage() const = 0;

	/// Get the name of the shader module
	virtual const std::string& name() const = 0;

	/// Get the entry point of the shader module
	virtual const std::string& entryPoint() const = 0;

	/// Get a list of the input attribute definitions required by this shader
	virtual std::span<const AttributeBindingLayout> inputAttributeBindingLayout() const = 0;

	/// Get a list of the out attribute definitions produced by this shader
	virtual std::span<const AttributeBindingLayout> outputAttributeBindingLayout() const = 0;

	/// Get a list of descriptor binding definitions required by this shader
	virtual std::span<const DescriptorBindingLayout> descriptorBindingLayout() const = 0;
};

} // namespace Coral

#endif // !CORAL_SHADERMODULE_HPP
