#ifndef CORAL_SHADERGRAPH_COMPILERGLSL_HPP
#define CORAL_SHADERGRAPH_COMPILERGLSL_HPP

#include <Coral/ShaderLanguage/Compiler.hpp>

#include <optional>
#include <unordered_map>

namespace Coral::ShaderLanguage
{

class CompilerGLSL : public Compiler
{
public:

	Compiler& addShaderModule(Coral::ShaderStage stage, const ShaderModule& shader) override;

	Compiler& addUniformBlockOverride(uint32_t binding, std::string_view name, const UniformBlockDefinition& override) override;

	Compiler& addInputAttributeBindingLocation(uint32_t location, std::string_view name) override;

	Compiler& addOutputAttributeBindingLocation(uint32_t location, std::string_view name) override;

	Compiler& setDefaultUniformBlockName(std::string_view name) override;

	std::expected<Result, Error> compile() override;

private:

	std::string format(const ShaderGraph::ConstantExpression<float>& expr);

	std::string format(const ShaderGraph::ConstantExpression<int>& expr);

	std::string format(const ShaderGraph::ConstantExpression<bool>& expr);

	std::string format(const ShaderGraph::InputAttributeExpression& expr);

	std::string format(const ShaderGraph::OutputAttributeExpression& expr);

	std::string format(const ShaderGraph::OperatorExpression& expr);

	std::string format(const ShaderGraph::ParameterExpression& expr);

	std::string format(const ShaderGraph::NativeFunctionExpression& expr);

	std::string format(const ShaderGraph::ConstructorExpression& expr);

	std::string format(const ShaderGraph::CastExpression& expr);

	std::string format(const ShaderGraph::SwizzleExpression& expr);

	std::string format(const ShaderGraph::ConditionalExpression& expr);

	std::string format(const ShaderGraph::Expression* expr);

	std::string resolve(const ShaderGraph::Expression& expr);

	std::optional<uint32_t> findUniformBinding(std::string_view parameterName);

	bool shouldHaveVariableAssignment(const ShaderGraph::Expression& epxr);

	std::string buildVariableAssignments(const ShaderGraph::Expression& expr);

	std::string formatFunctionArgumentList(const std::vector<const ShaderGraph::Expression*>& expressions);

	void buildVariableNames(const ShaderModule& shader);

	std::string buildInputAttributeDefinitionsString(const ShaderModule& shader);

	std::string buildOutputAttributeDefinitionsString(const ShaderModule& shader);

	std::string buildMainFunctionString(const ShaderModule& shader);

	std::string buildUniformBlocksString(const ShaderModule& shader);

	void createUniformBlockDefinitions();

	bool createAttributeBindings();

	struct ShaderStageAttributeBindings
	{	
		std::unordered_map<std::string, uint32_t> inputAttributes;
		std::unordered_map<std::string, uint32_t> outputAttributes;
	};

	const ShaderModule* mVertexShader{ nullptr };
	const ShaderModule* mFragmentShader{ nullptr };

	std::map<uint32_t, Coral::DescriptorBindingLayout> mDescriptorBindings;

	std::unordered_map<std::string, uint32_t> mInputAttributeBindingOverrides;

	std::unordered_map<std::string, uint32_t> mOutputAttributeBindingOverrides;

	std::unordered_map<const ShaderModule*, ShaderStageAttributeBindings> mShaderStageAttributeBindingsLookUp;

	std::unordered_map<const ShaderGraph::Expression*, std::string> mNameLookUp;

	std::string mDefaultUniformBlockName{ "Uniforms" };

	uint32_t mDefaultDescriptorSet{ 0 };

	uint32_t mVarCounter{ 0 };
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_SHADERGRAPHCOMPILERGLSL_HPP
