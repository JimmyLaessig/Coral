#ifndef CORAL_SHADERGRAPH_COMPILERGLSL_HPP
#define CORAL_SHADERGRAPH_COMPILERGLSL_HPP

#include <Coral/ShaderGraph/Compiler.hpp>

#include <unordered_map>

namespace Coral::ShaderGraph
{

class CompilerGLSL : public Compiler
{
public:

	virtual Compiler& addShader(Coral::ShaderStage stage, const Shader& shader) override;

	virtual Compiler& addUniformBlockOverride(uint32_t binding, std::string_view name, const UniformBlockDefinition & override) override;

	virtual Compiler& setDefaultUniformBlockName(std::string_view name) override;


	virtual std::optional<Result> compile() override;
	
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

	std::string format(const ShaderGraph::ConditionalExpression& expr);

	std::string format(const ShaderGraph::Expression& expr);

	std::string getRefName(ShaderGraph::NodePtr node);

	std::optional<uint32_t> findUniformBinding(std::string_view parameterName);

	std::string buildFunctionArgumentList(const std::vector<NodePtr>& expressions);

	std::string buildInputAttributeDefinitionsString(const Shader& shader);

	std::string buildOutputAttributeDefinitionsString(const Shader& shader);

	std::string buildMainFunctionString(const Shader& shader);

	std::string buildUniformBlocksString(const Shader& shader);

	bool createUniformBlockDefinitions();

	bool createAttributeLocationDefinitions();

	struct ShaderStageAttributeBindings
	{
		std::map<const ShaderGraph::AttributeExpression*, uint32_t> inputAttributes;
		std::map<const ShaderGraph::AttributeExpression*, uint32_t> outputAttributes;
	};

	const Shader* mVertexShader{ nullptr };
	const Shader* mFragmentShader{ nullptr };
	
	std::map<uint32_t, Coral::DescriptorBindingLayout> mDescriptorBindings;

	std::unordered_map<ShaderGraph::NodePtr, std::string> mNameLookUp;

	std::unordered_map<const Shader*, ShaderStageAttributeBindings> mShaderStageAttributeBindingsLookUp;

	std::string mDefaultUniformBlockName{ "Uniforms" };

	uint32_t mDefaultDescriptorSet{ 0 };

	uint32_t mVarCounter{ 0 };
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_SHADERGRAPHCOMPILERGLSL_HPP
