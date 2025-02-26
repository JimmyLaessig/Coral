#ifndef CORAL_SHADERGRAPH_COMPILER_HPP
#define CORAL_SHADERGRAPH_COMPILER_HPP

#include <Coral/ShaderGraph/ShaderGraph.hpp>
#include <Coral/ShaderModule.hpp>

#include <map>
#include <optional>
#include <string>

namespace Coral::ShaderLanguage::ShaderGraph
{

//struct UniformTextureDefinition
//{
//	/// The shader binding of the texture
//	uint32_t binding{ 0 };
//
//	/// The name of the uniform block
//	std::string name;
//
//	/// The type of texture/sampler
//	ShaderTypeId textureType;
//};


struct CompilerResult
{
	std::string vertexShader;
	std::string fragmentShader;
};


class Compiler
{
public:

	/// Set the shader program to compile
	virtual Compiler& setShaderProgram(const ShaderProgram& shaderProgram) = 0;

	/// Force the shader to use a specific uniform block for parameters
	virtual Compiler& addUniformBlockOverride(const Coral::UniformBlockDefinition& override) = 0;

	/// Set the name of the default uniform block
	virtual Compiler& setDefaultUniformBlockName(std::string_view name) = 0;

	/// Compile the shader program
	virtual std::optional<CompilerResult> compile() = 0;

};

} // namespace Coral::Slang::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILER_HPP
