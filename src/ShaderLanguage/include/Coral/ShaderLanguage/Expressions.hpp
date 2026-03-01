#ifndef CORAL_SHADERLANGUAGE_EXPRESSIONS_HPP
#define CORAL_SHADERLANGUAGE_EXPRESSIONS_HPP

#include <Coral/ShaderLanguage/Expression.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Coral::ShaderLanguage
{

template<typename T>
class ExpressionBase : public Expression
{
public:
	using Expression::Expression;

	ExpressionType Type() const override { return T::ClassType; }

};


/// Class defining a constant scalar value expression
template<typename Scalar>
class ConstantExpression : public ExpressionBase<ConstantExpression<Scalar>>
{
public:

	constexpr static ValueType GetValueType() requires std::same_as<Scalar, int> { return ValueType::INT; }
	constexpr static ValueType GetValueType() requires std::same_as<Scalar, float> { return ValueType::FLOAT; }
	constexpr static ValueType GetValueType() requires std::same_as<Scalar, bool> { return ValueType::BOOL; }

	constexpr static ExpressionType GetClassType() requires std::same_as<Scalar, int> { return ExpressionType::CONSTANT_INT; }
	constexpr static ExpressionType GetClassType() requires std::same_as<Scalar, float> { return ExpressionType::CONSTANT_FLOAT; }
	constexpr static ExpressionType GetClassType() requires std::same_as<Scalar, bool> { return ExpressionType::CONSTANT_BOOL; }

	constexpr static ExpressionType ClassType = GetClassType();

	ConstantExpression(Scalar value)
		: ExpressionBase<ConstantExpression<Scalar>>(GetValueType(), {})
		, mValue(value)
	{
	}


	Scalar value() const { return mValue; }

private:



	Scalar mValue{ 0 };
};


enum class DefaultSemantics
{
	/// Set the position of a vertex in homogenous space. Every vertex shader must write out a parameter with this output.
	/*
	 * \Note: The position output value must be a 4-component float vector.
	 * \Note: The SV_POSITION attribute is only accessible in vertex shaders.
	 */
	SV_POSITION,

	/// Shader output that is used to override the z buffer value in the fragment shader.
	/*
	 * \Note: The depth output value must be a single float.
	 * \Note: the SV_DEPTH attribute is only accessible in fragment shaders.
	 */
	SV_DEPTH,
};


/// Class defining a shader input attrribute
class InputAttributeExpression : public ExpressionBase<InputAttributeExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::INPUT_ATTRIBUTE;

	InputAttributeExpression(const ValueType& type, uint32_t location, std::string_view name)
		: ExpressionBase(type, {})
		, mName(name)
		, mLocation(location)
	{
	}

	const std::string& name() const { return mName; }

	uint32_t location() const { return mLocation; }

	bool InlineIfPossible() const override { return true; }

private:

	uint32_t mLocation;
	std::string mName;
};


struct AttributeBinding
{
	std::string name;
	uint32_t location;
};

using AttributeBindingInfo = std::variant<AttributeBinding, DefaultSemantics>;

/// Class defining a shader output attribute
class OutputAttributeExpression : public ExpressionBase<OutputAttributeExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::OUTPUT_ATTRIBUTE;

	OutputAttributeExpression(ExpressionPtr input, uint32_t location, std::string_view name)
		: ExpressionBase(input->GetValueType(), { input })
		, mBindingInfo(AttributeBinding{ std::string(name), location })
	{
	}

	OutputAttributeExpression(ExpressionPtr input, DefaultSemantics attribute)
		: ExpressionBase(input->GetValueType(), { input })
		, mBindingInfo(attribute)
	{
	}

	const AttributeBindingInfo& BindingInfo() const { return mBindingInfo; }

	bool InlineIfPossible() const override { return true; }

private:

	AttributeBindingInfo mBindingInfo;
};


class UniformBufferExpression : public ExpressionBase<UniformBufferExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::UNIFORM_BUFFER;

	UniformBufferExpression(uint32_t location, std::string_view name)
		: ExpressionBase(ValueType::STRUCT, {})
		, mLocation(location)
		, mName(name)
	{}

	const std::string& name() const
	{
		return mName;
	}

	uint32_t location() const
	{
		return mLocation;
	}

private:

	uint32_t mLocation;

	std::string mName;

};


class UniformExpression : public ExpressionBase<UniformExpression>
{
public:

	constexpr static ExpressionType ClassType = ExpressionType::UNIFORM;

	UniformExpression(const ValueType& type, std::string_view name, std::shared_ptr<UniformBufferExpression> buffer)
		: ExpressionBase(type, { buffer })
		, mName(name)
	{
	}

	bool InlineIfPossible() const override { return true; }

	const std::string& name() const { return mName; }

private:

	std::string mName;
};


class SamplerExpression : public ExpressionBase<SamplerExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::SAMPLER;

	SamplerExpression(uint32_t location, std::string_view name)
		: ExpressionBase(ValueType::SAMPLER2D, {})
		, mName(name)
		, mLocation(location)
	{
	}

	const std::string& name() const { return mName; }

	uint32_t location() const { return mLocation; }

private:

	std::string mName;
	uint32_t mLocation;
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
class OperatorExpression : public ExpressionBase<OperatorExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::OPERATOR;

	OperatorExpression(ValueType outputType, ExpressionPtr lhs, Operator op, ExpressionPtr rhs)
		: ExpressionBase(outputType, { lhs, rhs })
		, mOperator(op)
	{
	}

	Operator getOperator() const { return mOperator; }

private:

	Operator mOperator;
};


/// Class defining a cast operator expression
class CastExpression : public ExpressionBase<CastExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::CAST;

	CastExpression(ValueType outputType, ExpressionPtr input)
		: ExpressionBase(outputType, { input })
	{
	}
};


enum class NativeFunction
{
	// Normalize a vector
	NORMALIZE, 
	// Calculate the dot product of two vectors
	DOT, 
	// Calculate the cross product of two vectors
	CROSS,
	// Calculate the length of a vector
	LENGTH,
	// Calculate the distance between two vectors
	DISTANCE,
	// Read from a sampler
	SAMPLE,
	// Size of a texture
	TEXTURE_SIZE,
};


/// Class defining a native function call in the shader graph
class NativeFunctionExpression : public ExpressionBase<NativeFunctionExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::NATIVE_FUNCTION;

	template<typename ...ExpressionPtrs>
	NativeFunctionExpression(ValueType valueType, NativeFunction function, ExpressionPtrs... inputs)
		: ExpressionBase(valueType, { inputs... })
		, mFunction(function)
	{
	}

	NativeFunction Function() const { return mFunction; }

private:

	NativeFunction mFunction;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public ExpressionBase<ConstructorExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::CONSTRUCTOR;

	ConstructorExpression(ValueType valueType)
		: ExpressionBase(valueType, {  })
	{
	}

	template<typename ...ExpressionPtrs>
	ConstructorExpression(ValueType valueType, ExpressionPtrs... inputs)
		: ExpressionBase(valueType, std::vector<ExpressionPtr>{ (inputs)... })
	{
	}
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
class SwizzleExpression : public ExpressionBase<SwizzleExpression>
{
public:
	constexpr static ExpressionType ClassType = ExpressionType::SWIZZLE;

	SwizzleExpression(ValueType outputType, Swizzle swizzle, ExpressionPtr input)
		: ExpressionBase(outputType, { input })
		, mSwizzle(swizzle)
	{
	}

	Swizzle swizzle() const { return mSwizzle; };

private:

	Swizzle mSwizzle;
};

} // namespace Coral::ShaderLanguage

#endif // !CORAL_SHADERLANGUAGE_EXPRESSIONS_HPP
