#include <Coral/ShaderGraph/ShaderGraph.hpp>

#include "Visitor.hpp"

#include <algorithm>
#include <unordered_set>

using namespace Coral::ShaderGraph;


ValueType
ExpressionBase::outputValueType() const
{ 
	return mOutputTypeId;
}


std::vector<ValueType>
ExpressionBase::inputValueTypes() const
{
	auto getOutputTypeId = [](Expression expression) { return std::visit([](auto expr)
	{
		return expr->outputValueType(); }, expression);
	};

	return mInputs | std::views::transform(getOutputTypeId) | std::ranges::to<std::vector<ValueType>>();
}


std::span<const Expression>
ExpressionBase::inputs() const
{
	return mInputs;
}


std::shared_ptr<AttributeExpression>
AttributeExpression::create(ValueType resultType, std::string_view name)
{
	std::shared_ptr<AttributeExpression> expr(new AttributeExpression(resultType));
	expr->mName = std::string(name.begin(), name.end());
	return expr;
}


std::shared_ptr<AttributeExpression>
AttributeExpression::create(Expression source, std::string_view name)
{
	auto resultType = std::visit([](auto expr) { return expr->outputValueType(); }, source);
	std::shared_ptr<AttributeExpression> expr(new AttributeExpression(resultType, source));
	expr->mName = std::string(name.begin(), name.end());
	return expr;
}


std::shared_ptr<ParameterExpression> 
ParameterExpression::create(ValueType resultType, std::string_view name)
{
	std::shared_ptr<ParameterExpression> expr(new ParameterExpression(resultType));
	expr->mName = std::string(name.begin(), name.end());
	return expr;
}


std::shared_ptr<OperatorExpression>
OperatorExpression::create(ValueType resultType, Operator op, Expression lhs, Expression rhs)
{
	std::shared_ptr<OperatorExpression> expr(new OperatorExpression(resultType, lhs, rhs));
	expr->mOperator = op;
	return expr;
}


std::shared_ptr<CastExpression>
CastExpression::create(ValueType resultType, Expression input)
{
	return std::shared_ptr<CastExpression>(new CastExpression(resultType, input));
}


std::shared_ptr<SwizzleExpression> 
SwizzleExpression::create(ValueType resultType, Swizzle swizzle, Expression input)
{
	std::shared_ptr<SwizzleExpression> expr(new SwizzleExpression(resultType, input));
	expr->mSwizzle = swizzle;
	return expr;
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

	std::vector<Expression> result(outputs.begin(), outputs.end());

	for (auto [_, outputs] : mOutputs)
	{
		for (auto input : outputs->inputs())
		{
			buildExpressionListRecursive(input, result);
		}
	}

	return std::views::reverse(result) | std::ranges::to<std::vector<Expression>>();
}


void
Program::addVertexShaderOutput(std::string_view name, Expression expression)
{
	if (!mVertexShader)
	{
		mVertexShader.emplace(Coral::ShaderStage::VERTEX);
	}

	mVertexShader->addOutput(name, expression);
}


void
Program::addFragmentShaderOutput(std::string_view name, Expression expression)
{
	if (!mFragmentShader)
	{
		mFragmentShader.emplace(Coral::ShaderStage::FRAGMENT);
	}

	mFragmentShader->addOutput(name, expression);
}


const ShaderModule*
Program::vertexShader() const
{
	if (mVertexShader)
	{
		return &*mVertexShader;
	}

	return nullptr;
}


const ShaderModule*
Program::fragmentShader() const
{
	if (mFragmentShader)
	{
		return &*mFragmentShader;
	}

	return nullptr;
}
