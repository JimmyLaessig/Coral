#ifndef CORAL_SHADERGRAPH_COMPILERGLSL_HPP
#define CORAL_SHADERGRAPH_COMPILERGLSL_HPP

#include <Coral/ShaderGraph/Compiler.hpp>

#include <unordered_map>

namespace Coral::ShaderGraph
{

class CompilerGLSL : public Compiler
{
public:

	virtual Compiler& setShaderProgram(const Program& shaderGraph) override;

	virtual Compiler& addUniformBlockOverride(uint32_t set, uint32_t binding, std::string_view name, const UniformBlockDefinition & override) override;

	virtual Compiler& setDefaultUniformBlockName(std::string_view name) override;

	virtual Compiler& setDefaultDescriptorSet(uint32_t set) override;

	virtual std::optional<CompilerResult> compile() override;
	
private:

	std::string format(const Constant<float>& expr);

	std::string format(const Constant<int>& expr);

	std::string format(const AttributeExpression& expr);

	std::string format(const OperatorExpression& expr);

	std::string format(const ParameterExpression& expr);

	std::string format(const NativeFunctionExpression& expr);

	std::string format(const ConstructorExpression& expr);

	std::string format(const CastExpression& expr);

	std::string format(const SwizzleExpression& expr);

	std::string format(const Expression& expr);

	std::string getRefName(const ExpressionBase& expr);

	std::string getRefName(const Constant<float>& expr);

	std::string getRefName(const Constant<int>& expr);

	std::string getRefName(const AttributeExpression& expr);

	std::string getRefName(const ParameterExpression& expr);

	std::string getRefName(const SwizzleExpression& expr);

	std::string getRefName(const Expression expr);

	std::optional<std::pair<uint32_t, uint32_t>> findUniformBlock(std::string_view parameterName);

	std::string buildFunctionArgumentList(std::span<const Expression> expressions);

	std::string buildInputAttributeDefinitionsString(const ShaderModule& shaderModule);

	std::string buildOutputAttributeDefinitionsString(const ShaderModule& shaderModule);

	std::string buildMainFunctionString(const ShaderModule& shaderModule);

	std::string buildUniformBlocksString();

	bool createUniformBlockDefinitions();

	bool createAttributeLocationDefinitions();

	

	struct ShaderStageAttributeBindings
	{
		std::map<const AttributeExpressionPtr, uint32_t> inputAttributes;
		std::map<const AttributeExpressionPtr, uint32_t> outputAttributes;
	};

	const Program* mShaderProgram{ nullptr };

	std::map<uint32_t, std::map<uint32_t, Coral::DescriptorBindingDefinition>> mDescriptorBindings;

	std::unordered_map<const ExpressionBase*, std::string> mNameLookUp;

	std::unordered_map<const ShaderModule*, ShaderStageAttributeBindings> mShaderStageAttributeBindingsLookUp;

	std::string mDefaultUniformBlockName{ "Uniforms" };

	uint32_t mDefaultDescriptorSet{ 0 };
};

} // namespace Coral::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILERGLSL_HPP
