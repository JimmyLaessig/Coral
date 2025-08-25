#include <Coral/ShaderLanguage/ShaderModule.hpp>

#include <algorithm>
#include <unordered_set>
#include <cassert>
#include <ranges>

using namespace Coral::ShaderLanguage;
using namespace Coral::ShaderLanguage::ShaderGraph;


template<typename T>
void
collectShaderModuleExpressionsRecursive(const Expression& expression, std::vector<const T*>& result, std::unordered_set<const T*>& visited)
{
	if (expression.typeId() == T::classTypeId())
	{
		if (visited.insert(static_cast<const T*>(&expression)).second)
		{
			result.push_back(static_cast<const T*>(&expression));
		}
	}

	for (const auto* input : expression.inputs())
	{
		collectShaderModuleExpressionsRecursive(*input, result, visited);
	}
}


std::vector<const InputAttributeExpression*>
ShaderModule::inputs() const
{
	std::vector<const InputAttributeExpression*> result;
	std::unordered_set<const InputAttributeExpression*> visited;
	for (const auto output : mOutputs)
	{
		for (auto input : output->inputs())
		{
			collectShaderModuleExpressionsRecursive(*input, result, visited);
		}
	}

	return result;
}


std::vector<const ParameterExpression*>
ShaderModule::parameters() const
{
	std::vector<const ParameterExpression*> result;
	std::unordered_set<const ParameterExpression*> visited;

	for (const auto output : mOutputs)
	{
		for (auto input : output->inputs())
		{
			collectShaderModuleExpressionsRecursive(*input, result, visited);
		}
	}

	return result;
}


std::vector<const OutputAttributeExpression*>
ShaderModule::outputs() const
{
	return mOutputs
		| std::views::transform([](auto node) { return node.get(); })
		| std::ranges::to<std::vector<const OutputAttributeExpression*>>();
}


void
buildExpressionListRecursive(const Expression& expression, std::vector<const Expression*>& expressions)
{
	expressions.push_back(&expression);

	for (auto input : expression.inputs())
	{
		buildExpressionListRecursive(*input, expressions);
	}
}


std::vector<const Expression*>
ShaderModule::buildExpressionList() const
{
	auto result = mOutputs
		| std::views::transform([](auto node) { return node.get(); })
		| std::ranges::to<std::vector<const Expression*>>();

	for (auto node : mOutputs)
	{
		for (auto input : node->inputs())
		{
			buildExpressionListRecursive(*input, result);
		}
	}

	std::reverse(result.begin(), result.end());
	
	return result;
}


void
ShaderModule::registerOutputAttribute(std::shared_ptr<OutputAttributeExpression> attribute)
{
	mOutputs.push_back(attribute);
}
