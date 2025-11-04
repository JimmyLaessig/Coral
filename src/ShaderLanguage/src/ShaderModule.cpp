#include <Coral/ShaderLanguage/ShaderModule.hpp>

#include <algorithm>
#include <cassert>
#include <ranges>
#include <set>
#include <unordered_set>

using namespace Coral::ShaderLanguage;


void
ShaderModule::addExpression(std::shared_ptr<Expression> ex)
{
	mInstructions.push_back(ex);
}


std::vector<const InputAttributeExpression*>
ShaderModule::inputs() const
{
	std::vector<const InputAttributeExpression*> result;
	std::set<const InputAttributeExpression*> visited;
	for (const auto instruction : mInstructions)
	{
		if (instruction->expressionType() == InputAttributeExpression::ClassType())
		{
			auto attr = static_cast<const InputAttributeExpression*>(instruction.get());
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const ParameterExpression*>
ShaderModule::parameters() const
{
	std::vector<const ParameterExpression*> result;
	std::set<const ParameterExpression*> visited;
	for (const auto instruction : mInstructions)
	{
		if (instruction->expressionType() == ParameterExpression::ClassType())
		{
			auto attr = static_cast<const ParameterExpression*>(instruction.get());
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const OutputAttributeExpression*>
ShaderModule::outputs() const
{
	std::vector<const OutputAttributeExpression*> result;
	std::set<const OutputAttributeExpression*> visited;

	for (const auto instruction : mInstructions)
	{
		if (instruction->expressionType() == OutputAttributeExpression::ClassType())
		{
			auto attr = static_cast<const OutputAttributeExpression*>(instruction.get());
			if (visited.insert(attr).second)
			{
				result.push_back(attr);
			}
		}
	}

	return result;
}


std::vector<const Expression*>
ShaderModule::expressionList() const
{
	return mInstructions 
		| std::views::transform(&std::shared_ptr<Expression>::get)
		| std::ranges::to<std::vector<const Expression*>>();
}
