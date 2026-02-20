#ifndef CORAL_SHADERGRAPH_COMPILERSPV_HPP
#define CORAL_SHADERGRAPH_COMPILERSPV_HPP

#include <Coral/ShaderLanguage/CompilerGLSL.hpp>


namespace Coral::ShaderLanguage
{

class CompilerSPV : public Compiler
{
public:

	/*Compiler& addUniformBlockOverride(uint32_t binding, std::string_view name, const CoUniformBlockDefinition& override) override;

	Compiler& addInputAttributeBindingLocation(uint32_t location, std::string_view name) override;

	Compiler& addOutputAttributeBindingLocation(uint32_t location, std::string_view name) override;

	Compiler& setDefaultUniformBlockName(std::string_view name) override;*/

	std::expected<Result, Error> Compile(const ShaderModule& vertexShader, const ShaderModule& fragmentShader) override;

	Result GetCompiledShaderSourceGLSL() const;

private:

	std::expected<Result, Error> CompileSPV(const Result& shaderSourceGLSL);

	CompilerGLSL mCompilerGLSL;

	Result mShaderSourceGLSL;
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_COMPILERSPV_HPP
