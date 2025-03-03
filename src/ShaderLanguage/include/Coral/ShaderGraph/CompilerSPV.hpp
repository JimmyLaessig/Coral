#ifndef CORAL_SHADERGRAPH_COMPILERSPV_HPP
#define CORAL_SHADERGRAPH_COMPILERSPV_HPP

#include <Coral/ShaderGraph/CompilerGLSL.hpp>


namespace Coral::ShaderGraph
{

class CompilerSPV : public Compiler
{
public:

	virtual Compiler& setShaderProgram(const Program& shaderProgram) override;

	virtual Compiler& addUniformBlockOverride(uint32_t set, uint32_t binding, std::string_view name, const UniformBlockDefinition & override) override;

	virtual Compiler& setDefaultUniformBlockName(std::string_view name) override;

	virtual Compiler& setDefaultDescriptorSet(uint32_t set) override;

	virtual std::optional<CompilerResult> compile() override;

private:

	CompilerGLSL mCompilerGLSL;
};

} // namespace Coral::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILERSPV_HPP
