#ifndef CORAL_SHADERLANGUAGE_SHADERGRAPH_SHADERGRAPH_HPP
#define CORAL_SHADERLANGUAGE_SHADERGRAPH_SHADERGRAPH_HPP

#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Coral::ShaderLanguage::ShaderGraph
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


class Expression;
using ExpressionPtr = std::shared_ptr<Expression>;


/// Base class for a shader graph expression
class Expression : public std::enable_shared_from_this<Expression>
{
public:

	// Unique id for each expression
	enum class TypeId
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

	Expression(ValueType outputValueType, const std::vector<ExpressionPtr>& inputs)
		: mOutputValueType(outputValueType)
		, mInputs(inputs)
	{
	}

	virtual ~Expression() = default;

	// Get the unique type id of the expression
	virtual TypeId typeId() const = 0;

	// Get the value type of the output of this expression
	ValueType outputValueType() const { return mOutputValueType; }

	// Get the list of inputs this expression
	std::vector<const Expression*> inputs() const
	{
		return mInputs
			| std::views::transform([](auto expr) { return expr.get(); })
			| std::ranges::to<std::vector<const Expression*>>();
	}

private:

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

	static TypeId classTypeId() requires std::is_same_v<Scalar, float>
	{
		return TypeId::CONSTANT_FLOAT;
	}

	static TypeId classTypeId() requires std::is_same_v<Scalar, int>
	{
		return TypeId::CONSTANT_INT;
	}

	static TypeId classTypeId() requires std::is_same_v<Scalar, bool>
	{
		return TypeId::CONSTANT_BOOL;
	}

	TypeId typeId() const override { return classTypeId(); }

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

	static TypeId classTypeId() { return TypeId::INPUT_ATTRIBUTE; }

	TypeId typeId() const override { return classTypeId(); }

	const std::string& attribute() const { return mName; }

private:

	std::string mName{};
};

enum class DefaultAttribute
{
	/// Set the position of a vertex in homogenous space. Every vertex shader must write out a parameter with this output.
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


using Attribute = std::variant<DefaultAttribute, std::string>;


/// Class defining a shader output attribute
class OutputAttributeExpression : public Expression
{
public:

	OutputAttributeExpression(DefaultAttribute attribute, std::shared_ptr<Expression> input)
		: Expression(input->outputValueType(), { input })
		, mAttribute(attribute)
	{
	}

	OutputAttributeExpression(std::string_view name, std::shared_ptr<Expression> input)
		: Expression(input->outputValueType(), { input })
		, mAttribute(std::string(name))
	{
	}

	static TypeId classTypeId() { return TypeId::OUTPUT_ATTRIBUTE; }

	TypeId typeId() const override { return classTypeId(); }

	const Attribute& attribute() const { return mAttribute; }

private:

	Attribute mAttribute{};
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

	static TypeId classTypeId() { return TypeId::PARAMETER; }

	TypeId typeId() const override { return classTypeId(); }

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
class OperatorExpression : public Expression
{
public:

	OperatorExpression(ValueType outputType, ExpressionPtr lhs, Operator op, ExpressionPtr rhs)
		: Expression(outputType, { lhs, rhs })
		, mOperator(op)
	{
	}

	Operator getOperator() const { return mOperator; }

	static TypeId classTypeId() { return TypeId::OPERATOR; }

	TypeId typeId() const override { return classTypeId(); }

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

	static TypeId classTypeId() { return TypeId::CAST; }

	TypeId typeId() const override { return classTypeId(); }
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

	static TypeId classTypeId() { return TypeId::NATIVE_FUNCTION; }

	TypeId typeId() const override { return classTypeId(); }

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

	static TypeId classTypeId() { return TypeId::CONSTRUCTOR; }

	TypeId typeId() const override { return classTypeId(); }
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

	static TypeId classTypeId() { return TypeId::SWIZZLE; }

	TypeId typeId() const override { return classTypeId(); }

	Swizzle swizzle() const { return mSwizzle; };

private:

	Swizzle mSwizzle;
};


/// Class defining a conditional operation in the shader graph
class ConditionalExpression : public Expression
{
public:
	ConditionalExpression(ExpressionPtr condition, ExpressionPtr ifBranch, ExpressionPtr elseBranch)
		: Expression(ifBranch->outputValueType(), { condition, ifBranch, elseBranch })
	{
	}

	static TypeId classTypeId() { return TypeId::CONDITIONAL; }

	TypeId typeId() const override { return classTypeId(); }
};

} // namespace Coral::ShaderLanguage::ShaderGraph

#endif // !CORAL_SHADERLANGUAGE_SHADERGRAPH_SHADERGRAPH_HPP
