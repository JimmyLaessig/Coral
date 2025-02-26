#ifndef CORAL_SHADERGRAPH_COMPILERGLSL_HPP
#define CORAL_SHADERGRAPH_COMPILERGLSL_HPP

#include <Coral/ShaderGraph/Compiler.hpp>

#include <unordered_map>

namespace Coral::ShaderLanguage::ShaderGraph
{

class CompilerGLSL : public Compiler
{
public:

	virtual Compiler& setShaderProgram(const ShaderProgram& shaderProgram) override;

	virtual Compiler& addUniformBlockOverride(const UniformBlockDefinition & override) override;

	virtual Compiler& setDefaultUniformBlockName(std::string_view name) override;

	virtual std::optional<CompilerResult> compile() override;
	
private:

	std::string format(const ShaderGraph::Constant<float>& expr);

	std::string format(const ShaderGraph::Constant<int>& expr);

	std::string format(const ShaderGraph::AttributeExpression& expr);

	std::string format(const ShaderGraph::OperatorExpression& expr);

	std::string format(const ShaderGraph::ParameterExpression& expr);

	std::string format(const ShaderGraph::NativeFunctionExpression& expr);

	std::string format(const ShaderGraph::ConstructorExpression& expr);

	std::string format(const ShaderGraph::CastExpression& expr);

	std::string format(const ShaderGraph::SwizzleExpression& expr);

	std::string format(const ShaderGraph::Expression& expr);

	std::string getRefName(const ShaderGraph::ExpressionBase& expr);

	std::string getRefName(const ShaderGraph::Constant<float>& expr);

	std::string getRefName(const ShaderGraph::Constant<int>& expr);

	std::string getRefName(const ShaderGraph::AttributeExpression& expr);

	std::string getRefName(const ShaderGraph::ParameterExpression& expr);

	std::string getRefName(const ShaderGraph::SwizzleExpression& expr);

	std::string getRefName(const ShaderGraph::Expression expr);

	std::string buildFunctionArgumentList(std::span<const ShaderGraph::Expression> expressions);

	std::string buildInputAttributeDefinitionsString(const ShaderGraph::ShaderModule& shaderModule);

	std::string buildOutputAttributeDefinitionsString(const ShaderGraph::ShaderModule& shaderModule);

	std::string buildMainFunctionString(const ShaderGraph::ShaderModule& shaderModule);

	std::string buildUniformBlocksString();

	bool createUniformBlockDefinitions();

	bool createAttributeLocationDefinitions();

	struct ShaderStageAttributeBindings
	{
		std::map<const ShaderGraph::AttributeExpressionPtr, uint32_t> inputAttributes;
		std::map<const ShaderGraph::AttributeExpressionPtr, uint32_t> outputAttributes;
	};

	const ShaderGraph::ShaderProgram* mShaderProgram{ nullptr };

	std::unordered_map<uint32_t, Coral::DescriptorBindingDefinition> mDescriptorBindings;

	std::unordered_map<const ShaderGraph::ExpressionBase*, std::string> mNameLookUp;

	std::unordered_map<const ShaderGraph::ShaderModule*, ShaderStageAttributeBindings> mShaderStageAttributeBindingsLookUp;

	std::string mDefaultUniformBlockName{ "Uniforms" };
};

} // namespace Coral::Slang::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILERGLSL_HPP
