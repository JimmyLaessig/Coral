#ifndef CORAL_SHADERGRAPH_COMPILERSPV_HPP
#define CORAL_SHADERGRAPH_COMPILERSPV_HPP

#include <Coral/ShaderGraph/CompilerGLSL.hpp>


namespace Coral::ShaderLanguage::ShaderGraph
{

class CompilerSPV : public Compiler
{
public:

	virtual Compiler& setShaderProgram(const ShaderGraph::ShaderProgram& shaderProgram) override;

	virtual Compiler& addUniformBlockOverride(const UniformBlockDefinition & override) override;

	virtual Compiler& setDefaultUniformBlockName(std::string_view name) override;

	virtual std::optional<CompilerResult> compile() override;

private:

	CompilerGLSL mCompilerGLSL;
};

} // namespace Coral::Slang::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILERSPV_HPP
