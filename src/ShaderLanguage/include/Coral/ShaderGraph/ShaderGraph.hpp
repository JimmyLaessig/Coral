#ifndef CORAL_SHADERGRAPH_HPP
#define CORAL_SHADERGRAPH_HPP

#include <Coral/Coral.hpp>

#include <concepts>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace Coral::ShaderGraph
{

enum class ValueType
{
	/// Conditional type
	BOOL,
	/// 32-bit signed integer
	INT,
	/// 2-component 32-bit signed integer vector
	INT2,
	/// 3-component 32-bit signed integer vector
	INT3,
	/// 4-component 32-bit signed integer vector
	INT4,
	/// 32-bit single-precision floating-point number
	FLOAT,
	/// 2-component 32-bit single-precision floating-point vector
	FLOAT2,
	/// 3-component 32-bit single-precision floating-point vector
	FLOAT3,
	/// 4-component 32-bit single-precision floating-point vector
	FLOAT4,
	/// 3x3 32-bit single-precision floating-point matrix
	FLOAT3X3,
	/// 4x4 32-bit single-precision floating-point matrix
	FLOAT4X4,
	/// 2D texture sampler
	SAMPLER2D,
};


class Node;
using NodePtr = std::shared_ptr<Node>;


/// Base class for a shader graph expression
class ExpressionBase
{
public:

	/// Get the ShaderTypeId of the expression's output
	ValueType outputValueType() const;

	NodePtr node() const;

	const std::vector<NodePtr>& inputs() const;

protected:

	ExpressionBase(ValueType outputValueTypeId, NodePtr NodePtr);

private:

	ValueType mOutputValueType;

	std::weak_ptr<Node> mNode;
};


/// Defines a constant scalar value expression
template<typename Scalar>
class Constant;


/// Defines a float scalar value expression
template<>
class Constant<float> : public ExpressionBase
{
public:

	Constant(float value, NodePtr NodePtr)
		: ExpressionBase(ValueType::FLOAT, NodePtr)
		, mValue(value)
	{
		mValue = value;
	}

	float value() const { return mValue; }

private:

	float mValue{ 0 };
};


/// Defines a int scalar value expression
template<>
class Constant<int> : public ExpressionBase
{
public:

	Constant(int value, NodePtr NodePtr)
		: ExpressionBase(ValueType::INT, NodePtr)
		, mValue(value)
	{
		mValue = value;
	}

	int value() const { return mValue; }

private:

	int mValue{ 0 };
};


enum class DefaultSemantics
{
	/// Set the position of a vertex in homogenous space. Every vertex shader must write out a parameter with this semantic.
	/*
		* \Note: The position output value must be a 4-component float vector.
		* \Note: The POSITION semantic is only available in vertex shaders.
		*/
	POSITION,
	/// Shader output that is used to override the z buffer value in the fragment shader.
	/*
		* \Note: The depth output value must be a single float.  
		* \Note: the DEPTH semantic is only in fragment shaders.
		*/
	DEPTH
};

using AttributeSemantics = std::variant<DefaultSemantics, std::string>;

/// Defines a shader attribute value
/*
	* 
	*/
class AttributeExpression : public ExpressionBase
{
public:

	// Create a new shader input attribute
	AttributeExpression(ValueType outputType, std::string_view name, NodePtr NodePtr);

	// Create a new shader output attribute
	AttributeExpression(AttributeSemantics semantics, NodePtr NodePtr);

	const AttributeSemantics& semantics() const;

private:

	AttributeSemantics mSemantics;
};


/// Defines a shader parameter value (also known as uniform)
class ParameterExpression : public ExpressionBase
{
public:

	// Create a new shader input attribute
	ParameterExpression(ValueType outputType, std::string_view name, NodePtr NodePtr);

	const std::string& name() const;

private:

	std::string mName;
};


enum class Operator
{
	// '*' operator
	MULTIPLY,
	// '/' operator
	DIVIDE,
	// '+' operator
	ADD,
	// '-' operator
	SUBTRACT,
	// '>' operator
	GREATER,
	// '<' operator
	LESS,
	// '==' operator
	EQUAL, 
	// '>=' operator
	GREATER_OR_EQUAL,
	// '<=' operator
	LESS_OR_EQUAL,
	// '!=' operator
	NOT_EQUAL
};


/// Class defining an arithmetic operator expression
class OperatorExpression : public ExpressionBase
{
public:

	OperatorExpression(ValueType outputType, Operator op, NodePtr NodePtr);

	Operator getOperator() const;

private:
	
	Operator mOperator{ Operator::ADD };
};


/// Class defining a cast operator expression
class CastExpression : public ExpressionBase
{
public:

	CastExpression(ValueType outputValueType, NodePtr NodePtr);

};


/// Class defining a native function call in the shader graph
class NativeFunctionExpression : public ExpressionBase
{
public:

	NativeFunctionExpression(ValueType outputValueType, std::string_view name, NodePtr NodePtr);

	const std::string& functionName() const;

private:

	std::string mFunctionName;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public ExpressionBase
{
public:

	ConstructorExpression(ValueType outputValueType, NodePtr NodePtr);

};


enum class Swizzle
{
	X,
	Y,
	Z,
	W,
	XY,
	XYZ,
};


class SwizzleExpression : public ExpressionBase
{
public:

	SwizzleExpression(ValueType resultType, Swizzle swizzle, NodePtr NodePtr);

	Swizzle swizzle() const;

private:

	Swizzle mSwizzle{ Swizzle::X };
};


class ConditionalExpression : public ExpressionBase
{
public:
	ConditionalExpression(ValueType outputValueType, NodePtr NodePtr);

};


using Expression = std::variant<Constant<float>,
								Constant<int>,
								AttributeExpression,
								ParameterExpression,
								OperatorExpression,
								NativeFunctionExpression,
								ConstructorExpression,
								CastExpression,
								SwizzleExpression,
								ConditionalExpression>;

/// 
class Node
{
public:

	/// Get the expression of the NodePtr
	const Expression& expression() const
	{
		return mExpression;
	}

	/// Get the inputs of the expression
	const std::vector<NodePtr>& inputs() const
	{
		return mInputs;
	}

	ValueType outputValueType() const;

	static NodePtr createConstant(float value);

	static NodePtr createConstant(int value);

	static NodePtr createAttribute(ValueType valueType, std::string_view name);

	static NodePtr createAttribute(NodePtr input, AttributeSemantics identifier);

	static NodePtr createParameter(ValueType valueType, std::string_view name);

	static NodePtr createOperator(ValueType valueType, Operator op, NodePtr lhs, NodePtr rhs);

	static NodePtr createCast(ValueType valueType, NodePtr input);

	template<typename ...NodePtrs>
	static NodePtr createNativeFunction(ValueType valueType, std::string_view name, NodePtrs... inputs)
	{
		NodePtr node(new Node(inputs...));
		node->mExpression = NativeFunctionExpression(valueType, name, node);
		return node;
	}

	template<typename ...NodePtrs>
	static NodePtr createConstructor(ValueType valueType, NodePtrs... inputs)
	{
		NodePtr node(new Node(inputs...));
		node->mExpression = ConstructorExpression(valueType, node);
		return node;
	}

	static NodePtr createSwizzle(ValueType valueType, Swizzle swizzle, NodePtr input);

	static NodePtr createCondition(NodePtr condition, NodePtr ifBranch, NodePtr elseBranch);

private:

	template<typename ...NodePtrs>
	Node(NodePtrs... inputs)
	{
		(mInputs.push_back(inputs), ...);
	}

	Expression mExpression = Constant<float>(0.f, nullptr);

	std::vector<NodePtr> mInputs;
};

class Shader
{
public:


	/// Add a custom output attribute
	/*
	 * The corresponding shader input attribute of the next shader stage is available with the same name
	 */
	void addOutput(std::string_view attributeName, NodePtr NodePtr);

	/// Add a output attribute for a default semantic
	/**
	 * \Note: Attributes using one of the default semantics are not available in the next shader stage
	 */
	void addOutput(DefaultSemantics semantic, NodePtr NodePtr);

	/// Add an output attribute for a default semantic with an additional output name
	void addOutput(std::string_view attributeName, DefaultSemantics semantic, NodePtr NodePtr);

	/// Get the input attributes of the shader module
	std::vector<const AttributeExpression*> inputs() const;

	/// Get the parameters used by this shader module
	std::vector<const ParameterExpression*> parameters() const;

	/// Get the output attributes of the shader module
	std::vector<const AttributeExpression*> outputs() const;

	/// Build a flattened list of all shader expression
	std::vector<const Expression*> buildExpressionList() const;

private:

	std::vector<std::pair<AttributeSemantics, NodePtr>> mOutputs;
};

} // namespace Coral

#endif // !CORAL_SHADERGRAPH_HPP
