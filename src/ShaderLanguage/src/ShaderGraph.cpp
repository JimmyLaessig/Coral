#include <Coral/ShaderGraph/ShaderGraph.hpp>

#include "Visitor.hpp"

#include <algorithm>
#include <unordered_set>
#include <cassert>

using namespace Coral::ShaderGraph;

//-------------------------------------------------------------------------------------------------
// ExpressionBase
//-------------------------------------------------------------------------------------------------

ExpressionBase::ExpressionBase(ValueType outputValueType, NodePtr node)
	: mOutputValueType(outputValueType)
	, mNode(node)
{}


ValueType
ExpressionBase::outputValueType() const
{ 
	return mOutputValueType;
}


NodePtr
ExpressionBase::node() const
{
	if (auto node = mNode.lock())
	{
		return node;
	}

	assert(false);
	return nullptr;
}


const std::vector<NodePtr>&
ExpressionBase::inputs() const
{
	return node()->inputs();
}

//-------------------------------------------------------------------------------------------------
// AttributeExpression
//-------------------------------------------------------------------------------------------------

AttributeExpression::AttributeExpression(ValueType outputValueType, std::string_view name, NodePtr node)
	: ExpressionBase(outputValueType, node)
	, mName(name)
{
}


AttributeExpression::AttributeExpression(std::string_view name, NodePtr node)
	: ExpressionBase(node->inputs().front()->outputValueType(), node)
	, mName(name)
{
}


const std::string&
AttributeExpression::name() const
{
	return mName;
}

//-------------------------------------------------------------------------------------------------
// ParameterExpression
//-------------------------------------------------------------------------------------------------

ParameterExpression::ParameterExpression(ValueType outputValueType, std::string_view name, NodePtr node)
	: ExpressionBase(outputValueType, node)
	, mName(name)
{
}


const std::string&
ParameterExpression::name() const
{
	return mName;
}

//-------------------------------------------------------------------------------------------------
// OperatorExpression
//-------------------------------------------------------------------------------------------------

OperatorExpression::OperatorExpression(ValueType outputValueType, Operator op, NodePtr node)
	: ExpressionBase(outputValueType, node)
	, mOperator(op)
{

}


Operator
OperatorExpression::getOperator() const
{
	return mOperator;
}

//-------------------------------------------------------------------------------------------------
// CastExpression
//-------------------------------------------------------------------------------------------------

CastExpression::CastExpression(ValueType outputValueType, NodePtr node)
	: ExpressionBase(outputValueType, node)
{
}

//-------------------------------------------------------------------------------------------------
// NativeFunctionExpression
//-------------------------------------------------------------------------------------------------

NativeFunctionExpression::NativeFunctionExpression(ValueType outputValueType, std::string_view name, NodePtr node)
	: ExpressionBase(outputValueType, node)
	, mFunctionName(name)
{
}


const std::string& 
NativeFunctionExpression::functionName() const
{
	return mFunctionName;
}

//-------------------------------------------------------------------------------------------------
// ConstructorExpression
//-------------------------------------------------------------------------------------------------

ConstructorExpression::ConstructorExpression(ValueType outputValueType, NodePtr node)
	: ExpressionBase(outputValueType, node)
{
}

//-------------------------------------------------------------------------------------------------
// SwizzleExpression
//-------------------------------------------------------------------------------------------------

SwizzleExpression::SwizzleExpression(ValueType outputValueType, Swizzle swizzle, NodePtr node)
	: ExpressionBase(outputValueType, node)
	, mSwizzle(swizzle)
{
}


Swizzle
SwizzleExpression::swizzle() const
{
	return mSwizzle;
}

//-------------------------------------------------------------------------------------------------
// Node
//-------------------------------------------------------------------------------------------------

ValueType
Node::outputValueType() const
{
	return getOutputValueType(mExpression);
}


NodePtr
Node::createConstant(float value)
{
	auto node = std::make_shared<Node>();
	node->mExpression = Constant<float>(value, node);
	return node;
}


NodePtr
Node::createConstant(int value)
{
	auto node = std::make_shared<Node>();
	node->mExpression = Constant<int>(value, node);
	return node;
}


NodePtr
Node::createAttribute(ValueType valueType, std::string_view name)
{
	auto node = std::make_shared<Node>();
	node->mExpression = AttributeExpression(valueType, name, node);
	return node;
}


NodePtr
Node::createAttribute(NodePtr input, std::string_view name)
{
	auto node = std::make_shared<Node>(input);
	node->mExpression = AttributeExpression(name, node);
	return node;
}


NodePtr
Node::createParameter(ValueType valueType, std::string_view name)
{
	auto node = std::make_shared<Node>();
	node->mExpression = ParameterExpression(valueType, name, node);
	return node;
}


NodePtr
Node::createOperator(ValueType valueType, Operator op, NodePtr lhs, NodePtr rhs)
{
	auto node = std::make_shared<Node>(lhs, rhs);
	node->mExpression = OperatorExpression(valueType, op, node);
	return node;
}


NodePtr
Node::createCast(ValueType valueType, NodePtr input)
{
	auto node = std::make_shared<Node>(input);
	node->mExpression = CastExpression(valueType, node);
	return node;
}


NodePtr
Node::createSwizzle(ValueType valueType, Swizzle swizzle, NodePtr input)
{
	auto node = std::make_shared<Node>(input);
	node->mExpression = SwizzleExpression(valueType, swizzle, node);
	return node;
}


void
ShaderModule::addOutput(std::string_view name, NodePtr node)
{
	auto attribute = Node::createAttribute(node, name);

	auto iter = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& pair) { return pair.first == name; });

	if (iter != mOutputs.end())
	{
		iter->second = attribute;
	}
	else
	{
		mOutputs.emplace_back(name, attribute);
	}
}


template<typename T>
void
collectShaderModuleInputsRecursive(NodePtr node, std::vector<T*>& result, std::unordered_set<T*>& visited)
{
	auto visitor = Visitor
	{
		[&](auto&)
		{
			return true;
		},
		[&](T& expr)
		{
			if (!visited.contains(&expr))
			{
				result.push_back(&expr);
				visited.insert(&expr);
			}
			return false;
		}
	};

	bool recurse = std::visit(visitor, node->expression());

	if (!recurse)
	{
		return;
	}

	for (auto input : node->inputs())
	{
		collectShaderModuleInputsRecursive(input, result, visited);
	}
}


std::vector<const AttributeExpression*>
ShaderModule::inputs() const
{
	std::vector<const AttributeExpression*> result;
	std::unordered_set<const AttributeExpression*> visited;
	for (const auto [_, attr] : mOutputs)
	{
		for (auto input : attr->inputs())
		{
			collectShaderModuleInputsRecursive(input, result, visited);
		}
	}

	return result;
}


std::vector<const ParameterExpression*>
ShaderModule::parameters() const
{
	std::vector<const ParameterExpression*> result;
	std::unordered_set<const ParameterExpression*> visited;

	for (const auto [_, attr] : mOutputs)
	{
		for (auto input : attr->inputs())
		{
			collectShaderModuleInputsRecursive(input, result, visited);
		}
	}

	return result;
}


std::vector<const AttributeExpression*>
ShaderModule::outputs() const
{
	std::vector<const AttributeExpression*> result;
	for (const auto [_, node] : mOutputs)
	{
		std::visit(Visitor{
			[](auto&) { assert(false); },
			[&](const AttributeExpression& expr)
			{
				result.push_back(&expr);
			}
		}, node->expression());
	}

	return result;
}


void
buildExpressionListRecursive(NodePtr node, std::vector<const Expression*>& expressions)
{
	expressions.push_back(&node->expression());

	for (auto input : node->inputs())
	{
		buildExpressionListRecursive(input, expressions);
	}
}


std::vector<const Expression*>
ShaderModule::buildExpressionList() const
{
	auto result = mOutputs 
		| std::views::transform([](auto node) { return &node.second->expression(); })
		| std::ranges::to<std::vector<const Expression*>>();

	for (auto [_, node] : mOutputs)
	{
		for (auto input : node->inputs())
		{
			buildExpressionListRecursive(input, result);
		}
	}

	std::reverse(result.begin(), result.end());
	
	return result;
}


void
Program::addVertexShaderOutput(std::string_view name, NodePtr node)
{
	if (!mVertexShader)
	{
		mVertexShader.emplace(Coral::ShaderStage::VERTEX);
	}

	mVertexShader->addOutput(name, node);
}


void
Program::addFragmentShaderOutput(std::string_view name, NodePtr node)
{
	if (!mFragmentShader)
	{
		mFragmentShader.emplace(Coral::ShaderStage::FRAGMENT);
	}

	mFragmentShader->addOutput(name, node);
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
