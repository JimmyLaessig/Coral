#ifndef CORAL_SHADERLANGUAGE_COMPILER_HPP
#define CORAL_SHADERLANGUAGE_COMPILER_HPP

#include <Coral/ShaderLanguage/ShaderModule.hpp>

#include <Coral/ShaderModule.hpp>

#include <expected>
#include <map>
#include <string_view>

namespace Coral::ShaderLanguage
{

class Compiler
{
public:

	struct Result
	{
		std::string vertexShader;
		std::string fragmentShader;
	};

	struct Error
	{
		std::string message;
	};

	/// Add a the shader graph to compile
	virtual Compiler& addShaderModule(Coral::ShaderStage stage, const ShaderModule& shaderModule) = 0;

	/// Force the shader to use a specific uniform block for parameters
	virtual Compiler& addUniformBlockOverride(uint32_t binding, std::string_view name, const Coral::UniformBlockDefinition& override) = 0;

	// Force the shader to bind the named input attribute to the specific location
	virtual Compiler& addInputAttributeBindingLocation(uint32_t location, std::string_view name) = 0;

	// Force the shader to bind the named output attribute to the specific location
	virtual Compiler& addOutputAttributeBindingLocation(uint32_t location, std::string_view name) = 0;

	/// Set the name of the default uniform block
	virtual Compiler& setDefaultUniformBlockName(std::string_view name) = 0;

	/// Compile the shader program
	virtual std::expected<Result, Error> compile() = 0;
};

} // namespace Coral

#endif // !CORAL_SHADERLANGUAGE_COMPILER_HPP
