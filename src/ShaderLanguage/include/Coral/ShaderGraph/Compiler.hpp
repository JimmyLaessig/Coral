#ifndef CORAL_SHADERGRAPH_COMPILER_HPP
#define CORAL_SHADERGRAPH_COMPILER_HPP

#include <Coral/ShaderGraph/ShaderGraph.hpp>
#include <Coral/ShaderModule.hpp>

#include <map>
#include <optional>
#include <string>

namespace Coral::ShaderGraph
{


struct CompilerResult
{
	std::string vertexShader;
	std::string fragmentShader;
};


class Compiler
{
public:

	/// Set the shader program to compile
	virtual Compiler& setShaderProgram(const Program& shaderProgram) = 0;

	/// Force the shader to use a specific uniform block for parameters
	virtual Compiler& addUniformBlockOverride(uint32_t set, uint32_t binding, std::string_view name, const Coral::UniformBlockDefinition& override) = 0;

	/// Set the name of the default uniform block
	virtual Compiler& setDefaultUniformBlockName(std::string_view name) = 0;

	/// Set the default descriptor set of the default uniforms
	virtual Compiler& setDefaultDescriptorSet(uint32_t set) = 0;

	/// Compile the shader program
	virtual std::optional<CompilerResult> compile() = 0;

};

} // namespace Coral::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILER_HPP
