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

	std::expected<Result, Error> Compile(const ShaderModule& vertexShader, const ShaderModule& fragmentShader) override;

private:

	std::string format(const ConstantExpression<float>& expr);

	std::string format(const ConstantExpression<int>& expr);

	std::string format(const ConstantExpression<bool>& expr);

	std::string format(const InputAttributeExpression& expr);

	std::string format(const OutputAttributeExpression& expr);

	std::string format(const OperatorExpression& expr);

	std::string format(const UniformBufferExpression& expr);

	std::string format(const StructMemberExpression& expr);

	std::string format(const NativeFunctionExpression& expr);

	std::string format(const ConstructorExpression& expr);

	std::string format(const CastExpression& expr);

	std::string format(const SwizzleExpression& expr);

	std::string format(const ConditionalExpression& expr);

	std::string format(const Expression* expr);

	std::string resolve(const Expression& expr);

	std::string formatFunctionArgumentList(const std::vector<const Expression*>& expressions);

	bool createAttributeBindings();

	std::string buildInputAttributeDefinitionsString(const ShaderModule& shader);

	std::string buildOutputAttributeDefinitionsString(const ShaderModule& shader);

	std::string buildMainFunctionString(const ShaderModule& shader);

	std::string buildUniformBlocksString(const ShaderModule& shader);

	std::optional<uint32_t> findUniformBinding(std::string_view parameterName);

	void createUniformBlockDefinitions();



	struct ShaderStageAttributeBindings
	{	
		std::unordered_map<std::string, uint32_t> inputAttributes;
		std::unordered_map<std::string, uint32_t> outputAttributes;
	};

	const ShaderModule* mVertexShader{ nullptr };
	const ShaderModule* mFragmentShader{ nullptr };

	//std::map<uint32_t, CoDescriptorBindingInfo> mDescriptorBindings;

	std::unordered_map<std::string, uint32_t> mInputAttributeBindingOverrides;

	std::unordered_map<std::string, uint32_t> mOutputAttributeBindingOverrides;

	std::unordered_map<const ShaderModule*, ShaderStageAttributeBindings> mShaderStageAttributeBindingsLookUp;

	std::unordered_map<const Expression*, std::string> mNameLookUp;

	std::string mDefaultUniformBlockName{ "Uniforms" };

	uint32_t mDefaultDescriptorSet{ 0 };

	uint32_t mVarCounter{ 0 };
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_SHADERGRAPHCOMPILERGLSL_HPP
