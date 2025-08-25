#ifndef CORAL_SHADERGRAPH_COMPILERSPV_HPP
#define CORAL_SHADERGRAPH_COMPILERSPV_HPP

#include <Coral/ShaderLanguage/CompilerGLSL.hpp>


namespace Coral::ShaderLanguage
{

class CompilerSPV : public Compiler
{
public:

	struct SPVResult
	{
		Result glslResult;
		Result spvResult;
	};

	Compiler& addShaderModule(Coral::ShaderStage stage, const ShaderModule& shaderModulw) override;

	Compiler& addUniformBlockOverride(uint32_t binding, std::string_view name, const Coral::UniformBlockDefinition& override) override;

	Compiler& addInputAttributeBindingLocation(uint32_t location, std::string_view name) override;

	Compiler& addOutputAttributeBindingLocation(uint32_t location, std::string_view name) override;

	Compiler& setDefaultUniformBlockName(std::string_view name) override;

	std::expected<Result, Error> compile() override;

	std::expected<SPVResult, Error> compile2();
private:

	CompilerGLSL mCompilerGLSL;
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_COMPILERSPV_HPP
