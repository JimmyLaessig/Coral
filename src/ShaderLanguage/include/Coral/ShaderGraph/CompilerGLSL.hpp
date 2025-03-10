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

	std::optional<std::string> formatDefaultSemantics(const AttributeExpression& expr, ShaderStage shaderStage);

	std::string getRefName(NodePtr node);

	std::optional<std::pair<uint32_t, uint32_t>> findUniformBinding(std::string_view parameterName);

	std::string buildFunctionArgumentList(std::span<const NodePtr> expressions);

	std::string buildInputAttributeDefinitionsString(const ShaderModule& shaderModule);

	std::string buildOutputAttributeDefinitionsString(const ShaderModule& shaderModule);

	std::string buildMainFunctionString(const ShaderModule& shaderModule);

	std::string buildUniformBlocksString(const ShaderModule& shaderModule);

	bool createUniformBlockDefinitions();

	bool createAttributeLocationDefinitions();

	

	struct ShaderStageAttributeBindings
	{
		std::map<const AttributeExpression*, uint32_t> inputAttributes;
		std::map<const AttributeExpression*, uint32_t> outputAttributes;
	};

	const Program* mShaderProgram{ nullptr };

	std::map<uint32_t, std::map<uint32_t, Coral::DescriptorBindingDefinition>> mDescriptorBindings;

	std::unordered_map<NodePtr, std::string> mNameLookUp;

	std::unordered_map<const ShaderModule*, ShaderStageAttributeBindings> mShaderStageAttributeBindingsLookUp;

	std::string mDefaultUniformBlockName{ "Uniforms" };

	uint32_t mDefaultDescriptorSet{ 0 };

	uint32_t mVarCounter{ 0 };
};

} // namespace Coral::ShaderGraph

#endif // !CORAL_SHADERGRAPH_COMPILERGLSL_HPP
