#include <Coral/ShaderGraph/ShaderGraph.hpp>

#include "Visitor.hpp"

#include <cassert>
#include <algorithm>
#include <unordered_set>

using namespace Coral::ShaderLanguage::ShaderGraph;


ValueTypeId
ExpressionBase::outputShaderTypeId() const
{ 
	return mOutputTypeId;
}


std::vector<ValueTypeId>
ExpressionBase::inputTypeIds() const
{
	auto getOutputTypeId = [](Expression expression) { return std::visit([](auto expr)
	{
		return expr->outputShaderTypeId(); }, expression);
	};

	return mInputs | std::views::transform(getOutputTypeId) | std::ranges::to<std::vector>();
}


std::span<const Expression>
ExpressionBase::inputs() const
{
	return mInputs;
}


void
ShaderModule::addOutput(std::string_view name, Expression expression)
{
	auto iter = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& pair) { return pair.first == name; });

	if (iter != mOutputs.end())
	{
		iter->second = AttributeExpression::create(expression, name);
	}
	else
	{
		mOutputs.emplace_back(name, AttributeExpression::create(expression, name));
	}
}


template<typename T>
void
collectShaderModuleInputsRecursive(Expression ex, std::vector<T>& result, std::unordered_set<T>& visited)
{
	std::visit(Visitor{
		[&](auto expr) { for (auto input : expr->inputs()) { collectShaderModuleInputsRecursive(input, result, visited); } },
		[&](T expr) { if (!visited.contains(expr)) { result.push_back(expr); visited.insert(expr); } }
	}, ex);
}


std::vector<AttributeExpressionPtr>
ShaderModule::inputs() const
{
	std::vector<AttributeExpressionPtr> result;
	std::unordered_set<AttributeExpressionPtr> visited;
	for (const auto [_, attr] : mOutputs)
	{
		for (auto input : attr->inputs())
		{
			collectShaderModuleInputsRecursive(input, result, visited);
		}
	}

	return result;
}


std::vector<ParameterExpressionPtr>
ShaderModule::parameters() const
{
	std::vector<ParameterExpressionPtr> result;
	std::unordered_set<ParameterExpressionPtr> visited;

	for (const auto [_, attr] : mOutputs)
	{
		for (auto input : attr->inputs())
		{
			collectShaderModuleInputsRecursive(input, result, visited);
		}
	}

	return result;
}


std::vector<AttributeExpressionPtr>
ShaderModule::outputs() const
{
	std::vector<AttributeExpressionPtr> result;
	for (const auto [_, attr] : mOutputs)
	{
		result.push_back(attr);
	}

	return result;
}


void
buildExpressionListRecursive(const Expression expr, std::vector<Expression>& expressions)
{
	expressions.push_back(expr);

	auto inputs = std::visit(Visitor
	{
		[&](AttributeExpressionPtr) -> std::span<const Expression> { return {}; },
		[&](auto ex) { return ex->inputs(); },
	}, expr);

	for (auto input : inputs)
	{
		buildExpressionListRecursive(input, expressions);
	}
}


std::vector<Expression>
ShaderModule::buildExpressionList() const
{
	auto outputs = this->outputs();

	std::vector<ShaderGraph::Expression> result(outputs.begin(), outputs.end());

	for (auto [_, outputs] : mOutputs)
	{
		for (auto input : outputs->inputs())
		{
			buildExpressionListRecursive(input, result);
		}
	}

	return std::views::reverse(result) | std::ranges::to<std::vector>();
}


void
ShaderProgram::addVertexShaderOutput(std::string_view name, Expression expression)
{
	if (!mVertexShader)
	{
		mVertexShader.emplace(Coral::ShaderStage::VERTEX);
	}

	mVertexShader->addOutput(name, expression);
}


void
ShaderProgram::addFragmentShaderOutput(std::string_view name, Expression expression)
{
	if (!mFragmentShader)
	{
		mFragmentShader.emplace(Coral::ShaderStage::FRAGMENT);
	}

	mFragmentShader->addOutput(name, expression);
}


const ShaderModule*
ShaderProgram::vertexShader() const
{
	if (mVertexShader)
	{
		return &*mVertexShader;
	}

	return nullptr;
}


const ShaderModule*
ShaderProgram::fragmentShader() const
{
	if (mFragmentShader)
	{
		return &*mFragmentShader;
	}

	return nullptr;
}
