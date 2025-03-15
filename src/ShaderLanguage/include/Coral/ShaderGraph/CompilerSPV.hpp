#ifndef CORAL_SHADERGRAPH_COMPILERSPV_HPP
#define CORAL_SHADERGRAPH_COMPILERSPV_HPP

#include <Coral/ShaderGraph/CompilerGLSL.hpp>


namespace Coral::ShaderGraph
{

class CompilerSPV : public Compiler
{
public:

	virtual Compiler& addShader(Coral::ShaderStage stage, const Shader& shader) override;

	virtual Compiler& addUniformBlockOverride(uint32_t set, uint32_t binding, std::string_view name, const UniformBlockDefinition & override) override;

	virtual Compiler& setDefaultUniformBlockName(std::string_view name) override;

	virtual Compiler& setDefaultDescriptorSet(uint32_t set) override;

	virtual std::optional<Result> compile() override;

private:

	CompilerGLSL mCompilerGLSL;
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_COMPILERSPV_HPP
