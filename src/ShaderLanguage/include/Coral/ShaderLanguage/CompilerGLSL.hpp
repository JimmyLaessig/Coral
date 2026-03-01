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

	std::expected<Result, Error> Compile(const ShaderGraph& shader, ShaderStage stage) override;

private:

	std::string format(const ConstantExpression<float>& expr);

	std::string format(const ConstantExpression<int>& expr);

	std::string format(const ConstantExpression<bool>& expr);

	std::string format(const InputAttributeExpression& expr);

	std::string format(const OutputAttributeExpression& expr);

	std::string format(const OperatorExpression& expr);

	std::string format(const UniformBufferExpression& expr);

	std::string format(const UniformExpression& expr);

	std::string format(const NativeFunctionExpression& expr);

	std::string format(const ConstructorExpression& expr);

	std::string format(const CastExpression& expr);

	std::string format(const SwizzleExpression& expr);

	//std::string format(const ConditionalExpression& expr);

	std::string format(const SamplerExpression& expr);

	std::string format(const Expression* expr);

	std::string resolve(const Expression& expr);

	std::string formatFunctionArgumentList(const std::vector<const Expression*>& expressions);

	std::string buildInputAttributeDefinitionsString();

	std::string buildOutputAttributeDefinitionsString();

	std::string buildMainFunctionString();

	std::string buildUniformBlocksString();

	std::string buildSamplerString();

	std::vector<const Expression*> mInstructionsList;

	const ShaderGraph* mShader{ nullptr };

	std::unordered_map<const Expression*, std::string> mNameLookUp;

	uint32_t mDefaultDescriptorSet{ 0 };

	uint32_t mVarCounter{ 0 };
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_SHADERGRAPHCOMPILERGLSL_HPP
