#ifndef CORAL_SHADERGRAPH_COMPILERSPV_HPP
#define CORAL_SHADERGRAPH_COMPILERSPV_HPP

#include <Coral/ShaderLanguage/CompilerGLSL.hpp>


namespace Coral::ShaderLanguage
{

class CompilerSPV : public Compiler
{
public:

	std::expected<Result, Error> Compile(const ShaderGraph& shaderModule, ShaderStage stage) override;

	Result GetShaderSourceGLSL() const;

private:

	std::expected<Result, Error> CompileSPV(const Result& shaderSourceGLSL, ShaderStage stage);

	CompilerGLSL mCompilerGLSL;

	const ShaderGraph* mShaderModule{ nullptr };

	Result mShaderSourceGLSL;
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_COMPILERSPV_HPP
