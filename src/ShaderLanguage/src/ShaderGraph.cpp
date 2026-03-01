#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>
#include <set>
#include <unordered_set>

using namespace Coral::ShaderLanguage;


std::vector<const InputAttributeExpression*>
ShaderGraph::Inputs() const
{
	std::vector<const InputAttributeExpression*> result;
	std::set<const InputAttributeExpression*> visited;
	for (const auto instruction : mInstructions)
	{
		if (auto attr = instruction->Cast<InputAttributeExpression>())
		{
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const UniformBufferExpression*>
ShaderGraph::UniformBuffers() const
{
	std::vector<const UniformBufferExpression*> result;
	std::set<const UniformBufferExpression*> visited;
	for (const auto instruction : mInstructions)
	{
		if (auto attr = instruction->Cast<UniformBufferExpression>())
		{
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const OutputAttributeExpression*>
ShaderGraph::Outputs() const
{
	std::vector<const OutputAttributeExpression*> result;
	std::set<const OutputAttributeExpression*> visited;

	for (const auto instruction : mInstructions)
	{
		if (auto attr = instruction->Cast<OutputAttributeExpression>())
		{
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const Expression*>
ShaderGraph::ExpressionList() const
{
	return mInstructions 
		| std::views::transform([](auto expr) { return expr.get(); })
		| std::ranges::to<std::vector<const Expression*>>();
}


void
ShaderGraph::ReplaceExpressionImpl(std::shared_ptr<Expression> oldExpression, 
	                               std::shared_ptr<Expression> newExpression)
{
	assert(oldExpression->GetValueType() == newExpression->GetValueType());

	if (!sCurrentShaderModule)
	{
		return;
	}

	auto& v = sCurrentShaderModule->mInstructions;

	for (auto& expr : v)
	{
		if (expr == oldExpression)
		{
			expr = newExpression;
		}
		else
		{
			for (auto& input : expr->mInputs)
			{
				if (input == oldExpression)
				{
					input = newExpression;
				}
			}
		}
	}
}
