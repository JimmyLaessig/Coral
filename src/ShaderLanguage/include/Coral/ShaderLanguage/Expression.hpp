#ifndef CORAL_SHADERLANGUAGE_EXPRESSION_HPP
#define CORAL_SHADERLANGUAGE_EXPRESSION_HPP

#include <Coral/System.hpp>

#include <Coral/ShaderLanguage/Variable.hpp>

#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <unordered_map>

namespace Coral::ShaderLanguage
{

// Unique type of each expression
enum class ExpressionType
{
	// The expression is a `ConstantExpression<float>`
	CONSTANT_FLOAT,
	// The expression is a `ConstantExpression<int>`
	CONSTANT_INT,
	// The expression is a `ConstantExpression<bool>`
	CONSTANT_BOOL,
	// The expression is an `InputAttributeExpression`
	INPUT_ATTRIBUTE,
	// The expression is an `OutputtAttributeExpression`
	OUTPUT_ATTRIBUTE,
	// The expression is a `ParameterExpression`
	PARAMETER,
	// The expression is a `NativeFunctionExpression`
	NATIVE_FUNCTION,
	// The expression is a `ConstructorExpression`
	CONSTRUCTOR,
	// The expression is a `CastExpression`
	CAST,
	// The expression is a `SwizzleExpression`
	SWIZZLE,
	// The expression is a `ConditionalExpression`
	CONDITIONAL,
	// The expression is an `OperatorExpression`
	OPERATOR
};


class Expression;
using ExpressionPtr = std::shared_ptr<Expression>;

//class ExpressionTree : public std::enable_shared_from_this<ExpressionTree>
//{
//public: 
//	ExpressionTree();
//
//	const std::vector<std::shared_ptr<Expression>> expressions() const;
//
//private:
//
//	std::vector<std::shared_ptr<Expression>> mExpressions;
//};

/// Base class for a shader graph expression
class Expression
{
public:

	Expression(ValueType outputValueType, const std::vector<ExpressionPtr>& inputs);

	virtual ~Expression() = default;

	// Get the unique type id of the expression
	virtual ExpressionType expressionType() const = 0;

	// Get the value type of the output of this expression
	ValueType resultValueType() const;

	// Get the list of inputs this expression
	std::vector<const Expression*> inputs() const;

	bool inlined() const
	{
		return mInline;
	}

private:

	bool mInline{ false };

	ValueType mOutputValueType;

	std::vector<ExpressionPtr> mInputs;
};


/// Class defining a constant scalar value expression
template<typename Scalar>
class ConstantExpression : public Expression
{
public:

	ConstantExpression(Scalar value) requires std::is_same_v<Scalar, float>
		: Expression(ValueType::FLOAT, {})
		, mValue(value)
	{
	}

	ConstantExpression(Scalar value) requires std::is_same_v<Scalar, int>
		: Expression(ValueType::INT, {})
		, mValue(value)
	{
	}

	ConstantExpression(Scalar value) requires std::is_same_v<Scalar, bool>
		: Expression(ValueType::BOOL, {})
		, mValue(value)
	{
	}

	static ExpressionType ClassType() requires std::is_same_v<Scalar, float>
	{
		return ExpressionType::CONSTANT_FLOAT;
	}

	static ExpressionType ClassType() requires std::is_same_v<Scalar, int>
	{
		return ExpressionType::CONSTANT_INT;
	}

	static ExpressionType ClassType() requires std::is_same_v<Scalar, bool>
	{
		return ExpressionType::CONSTANT_BOOL;
	}

	ExpressionType expressionType() const override { return ClassType(); }

	Scalar value() const { return mValue; }

private:

	Scalar mValue{ 0 };
};


/// Class defining a shader input attrribute
class InputAttributeExpression : public Expression
{
public:

	InputAttributeExpression(const ValueType& type, std::string_view name)
		: Expression(type, {})
		, mName(name)
	{
	}

	static ExpressionType ClassType() { return ExpressionType::INPUT_ATTRIBUTE; }

	ExpressionType expressionType() const override { return ClassType(); }

	const std::string& attribute() const { return mName; }

private:

	std::string mName{};
};


/// Class defining a shader output attribute
class OutputAttributeExpression : public Expression
{
public:

	OutputAttributeExpression(const ValueType& type, std::string_view name)
		: Expression(type, {})
		, mAttribute(std::string(name))
	{
	}

	static ExpressionType ClassType() { return ExpressionType::OUTPUT_ATTRIBUTE; }

	ExpressionType expressionType() const override { return ClassType(); }

	const std::string& attribute() const { return mAttribute; }

private:

	std::string mAttribute{};
};


/// Class defining a shader parameter value (also known as uniform)
class ParameterExpression : public Expression
{
public:

	// Create a new shader input attribute
	ParameterExpression(ValueType outputType, std::string_view name)
		: Expression(outputType, {})
		, mName(name)
	{
	}

	const std::string& name() const { return mName; }

	static ExpressionType ClassType() { return ExpressionType::PARAMETER; }

	ExpressionType expressionType() const override { return ClassType(); }

private:

	std::string mName;
};

enum class Operator
{
	// '=' operator
	ASSIGNMENT,
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
class OperatorExpression : public Expression
{
public:

	OperatorExpression(ValueType outputType, ExpressionPtr lhs, Operator op, ExpressionPtr rhs)
		: Expression(outputType, { lhs, rhs })
		, mOperator(op)
	{
	}

	Operator getOperator() const { return mOperator; }

	static ExpressionType ClassType() { return ExpressionType::OPERATOR; }

	ExpressionType expressionType() const override { return ClassType(); }

private:

	Operator mOperator;
};


/// Class defining a cast operator expression
class CastExpression : public Expression
{
public:

	CastExpression(ValueType outputType, ExpressionPtr input)
		: Expression(outputType, { input })
	{
	}

	static ExpressionType ClassType() { return ExpressionType::CAST; }

	ExpressionType expressionType() const override { return ClassType(); }
};


/// Class defining a native function call in the shader graph
class NativeFunctionExpression : public Expression
{
public:

	template<typename ...ExpressionPtrs>
	NativeFunctionExpression(ValueType valueType, std::string_view name, ExpressionPtrs... inputs)
		: Expression(valueType, { inputs... })
		, mFunctionName(name)
	{
	}

	static ExpressionType ClassType() { return ExpressionType::NATIVE_FUNCTION; }

	ExpressionType expressionType() const override { return ClassType(); }

	const std::string& functionName() const { return mFunctionName; }

private:

	std::string mFunctionName;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public Expression
{
public:

	template<typename ...ExpressionPtrs>
	ConstructorExpression(ValueType valueType, ExpressionPtrs... inputs)
		: Expression(valueType, { inputs... })
	{
	}

	static ExpressionType ClassType() { return ExpressionType::CONSTRUCTOR; }

	ExpressionType expressionType() const override { return ClassType(); }
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

/// Class defining a swizzle operation in the shader graph
class SwizzleExpression : public Expression
{
public:

	SwizzleExpression(ValueType outputType, Swizzle swizzle, ExpressionPtr input)
		: Expression(outputType, { input })
		, mSwizzle(swizzle)
	{
	}

	static ExpressionType ClassType() { return ExpressionType::SWIZZLE; }

	ExpressionType expressionType() const override { return ClassType(); }

	Swizzle swizzle() const { return mSwizzle; };

private:

	Swizzle mSwizzle;
};


/// Class defining a conditional operation in the shader graph
class ConditionalExpression : public Expression
{
public:
	ConditionalExpression(ExpressionPtr condition, ExpressionPtr ifBranch, ExpressionPtr elseBranch)
		: Expression(ifBranch->resultValueType(), { condition, ifBranch, elseBranch })
	{
	}

	static ExpressionType ClassType() { return ExpressionType::CONDITIONAL; }

	ExpressionType expressionType() const override { return ClassType(); }
};

} // namespace Coral::ShaderLanguage

#endif // !CORAL_SHADERLANGUAGE_EXPRESSION_HPP
