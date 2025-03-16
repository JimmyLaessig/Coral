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
	return mNode.lock();
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
	, mSemantics(std::string(name))
{
}


AttributeExpression::AttributeExpression(AttributeSemantics semantics, NodePtr node)
	: ExpressionBase(node->inputs().front()->outputValueType(), node)
	, mSemantics(semantics)
{
}


const AttributeSemantics&
AttributeExpression::semantics() const
{
	return mSemantics;
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
// ConditionalExpression
//-------------------------------------------------------------------------------------------------

ConditionalExpression::ConditionalExpression(ValueType outputValueType, NodePtr node)
	: ExpressionBase(outputValueType, node)
{
}

//-------------------------------------------------------------------------------------------------
// Node
//-------------------------------------------------------------------------------------------------

ValueType
Node::outputValueType() const
{
	return std::visit([](const auto& ex) { return ex.outputValueType(); }, mExpression);
}


NodePtr
Node::createConstant(float value)
{
	NodePtr node(new Node());
	node->mExpression = Constant<float>(value, node);
	return node;
}


NodePtr
Node::createConstant(int value)
{
	NodePtr node(new Node());
	node->mExpression = Constant<int>(value, node);
	return node;
}


NodePtr
Node::createAttribute(ValueType valueType, std::string_view name)
{
	NodePtr node(new Node());
	node->mExpression = AttributeExpression(valueType, name, node);
	return node;
}


NodePtr
Node::createAttribute(NodePtr input, AttributeSemantics identifier)
{
	NodePtr node(new Node(input));
	node->mExpression = AttributeExpression(identifier, node);
	return node;
}


NodePtr
Node::createParameter(ValueType valueType, std::string_view name)
{
	NodePtr node(new Node());
	node->mExpression = ParameterExpression(valueType, name, node);
	return node;
}


NodePtr
Node::createOperator(ValueType valueType, Operator op, NodePtr lhs, NodePtr rhs)
{
	NodePtr node(new Node(lhs, rhs));
	node->mExpression = OperatorExpression(valueType, op, node);
	return node;
}


NodePtr
Node::createCast(ValueType valueType, NodePtr input)
{
	NodePtr node(new Node(input));
	node->mExpression = CastExpression(valueType, node);
	return node;
}


NodePtr
Node::createSwizzle(ValueType valueType, Swizzle swizzle, NodePtr input)
{
	NodePtr node(new Node(input));
	node->mExpression = SwizzleExpression(valueType, swizzle, node);
	return node;
}


NodePtr
Node::createCondition(NodePtr condition, NodePtr ifBranch, NodePtr elseBranch)
{
	NodePtr node(new Node(condition, ifBranch, elseBranch));
	assert(ifBranch->outputValueType() == elseBranch->outputValueType());
	assert(condition->outputValueType() == ValueType::BOOL);
	node->mExpression = ConditionalExpression(ifBranch->outputValueType(), node);
	return node;
}


void
Shader::addOutput(std::string_view attributeName, NodePtr node)
{
	AttributeSemantics identifier = std::string(attributeName);

	auto attribute = Node::createAttribute(node, identifier);

	auto iter = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& pair) { return pair.first == identifier; });

	if (iter != mOutputs.end())
	{
		iter->second = attribute;
	}
	else
	{
		mOutputs.emplace_back(identifier, attribute);
	}
}


void
Shader::addOutput(DefaultSemantics attr, NodePtr node)
{
	AttributeSemantics identifier = attr;
	auto attribute = Node::createAttribute(node, identifier);

	auto iter = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& pair) { return pair.first == identifier; });

	if (iter != mOutputs.end())
	{
		iter->second = attribute;
	}
	else
	{
		mOutputs.emplace_back(attr, attribute);
	}
}


void
Shader::addOutput(std::string_view attributeName, DefaultSemantics attribute, NodePtr node)
{
	addOutput(attributeName, node);
	addOutput(attribute, node);
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
Shader::inputs() const
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
Shader::parameters() const
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
Shader::outputs() const
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
Shader::buildExpressionList() const
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
