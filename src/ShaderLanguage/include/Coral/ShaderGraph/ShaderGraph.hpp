#ifndef CORAL_SHADERGRAPH_HPP
#define CORAL_SHADERGRAPH_HPP

#include <concepts>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include <Coral/Coral.hpp>

///
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

template<typename T>
concept ScalarTypes = requires(T t) { std::is_same_v<T, float> || std::is_same_v<T, int>; };

template<ScalarTypes Scalar>
class Constant;
class AttributeExpression;
class ParameterExpression;
class OperatorExpression;
class NativeFunctionExpression;
class ConstructorExpression;
class CastExpression;
class SwizzleExpression;

template<ScalarTypes Scalar>
using ConstantPtr				   = std::shared_ptr<Constant<Scalar>>;
using AttributeExpressionPtr	   = std::shared_ptr<AttributeExpression>;
using ParameterExpressionPtr	   = std::shared_ptr<ParameterExpression>;
using OperatorExpressionPtr		   = std::shared_ptr<OperatorExpression>;
using NativeFunctionExpressionPtr  = std::shared_ptr<NativeFunctionExpression>;
using ConstructorExpressionPtr	   = std::shared_ptr<ConstructorExpression>;
using CastExpressionPtr			   = std::shared_ptr<CastExpression>;
using SwizzleExpressionPtr		   = std::shared_ptr<SwizzleExpression>;


using Expression = std::variant<ConstantPtr<float>,
							    ConstantPtr<int>,
							    AttributeExpressionPtr,
							    ParameterExpressionPtr,
							    OperatorExpressionPtr,
							    NativeFunctionExpressionPtr,
							    ConstructorExpressionPtr,
							    CastExpressionPtr,
							    SwizzleExpressionPtr>;


/// Base class for a shader graph expression
class ExpressionBase
{
public:

	/// Get the ShaderTypeId of the expression's output
	ValueType outputValueType() const;

	/// Get the ShaderTypeIds of the expression's inputs
	std::vector<ValueType> inputValueTypes() const;

	/// Get the inputs of the expression
	std::span<const Expression> inputs() const;

protected:

	template<typename ...Expressions>
	ExpressionBase(ValueType outputTypeId, Expressions... inputs)
		: mOutputTypeId(outputTypeId)
	{
		(mInputs.push_back(inputs), ...);
	}

	void addInput(Expression expr)
	{
		mInputs.push_back(expr);
	}

private:

	ValueType mOutputTypeId;

	std::vector<Expression> mInputs;
};


/// Defines a constant scalar value expression
template<ScalarTypes Scalar>
class Constant : public ExpressionBase
{
public:
	static std::shared_ptr<Constant> create(Scalar value)
	{
		ValueType id;
		if constexpr (std::is_same_v<Scalar, float>)
		{
			id = ValueType::FLOAT;
		}
		else if constexpr (std::is_same_v<Scalar, int>)
		{
			id = ValueType::INT;
		}
		else
		{
			static_assert(false && "Unsupported Constant type");
		}

		std::shared_ptr<Constant> expr(new Constant(id));
		expr->mValue = value;
		return expr;
	}

	Scalar value() const { return mValue; }

private:

	using ExpressionBase::ExpressionBase;

	Scalar mValue{ 0 };
};


/// Defines a shader attribute value
class AttributeExpression : public ExpressionBase
{
public:

	// Create a new Vertex shader input attribute
	static std::shared_ptr<AttributeExpression> create(ValueType resultType, std::string_view name);

	// Create a new shader output attribute
	static std::shared_ptr<AttributeExpression> create(Expression source, std::string_view name);

	const std::string& name() const { return mName; }

private:

	using ExpressionBase::ExpressionBase;

	std::string mName;
};


/// Defines a mutable shader parameter value
class ParameterExpression : public ExpressionBase
{
public:

	static std::shared_ptr<ParameterExpression> create(ValueType resultType, std::string_view name);

	const std::string& name() const { return mName; }

private:

	using ExpressionBase::ExpressionBase;

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
	SUBTRACT
};


/// Class defining an arithmetic operator expression
class OperatorExpression : public ExpressionBase
{
public:

	using ExpressionBase::ExpressionBase;

	static std::shared_ptr<OperatorExpression> create(ValueType resultType, Operator op, Expression lhs, Expression rhs);

	Operator getOperator() const { return mOperator; }

private:

	Operator mOperator{ Operator::ADD };
};


/// Class defining a cast operator expression
class CastExpression : public ExpressionBase
{
public:

	static std::shared_ptr<CastExpression> create(ValueType resultType, Expression input);

private:

	using ExpressionBase::ExpressionBase;

};


/// Class defining a native function call in the shader graph
class NativeFunctionExpression : public ExpressionBase
{
public:

	template<typename ...Expressions>
	static inline std::shared_ptr<NativeFunctionExpression> create(ValueType resultType, 
																   std::string_view functionName, 
																   Expressions ...inputs)
	{
		std::shared_ptr<NativeFunctionExpression> expr(new NativeFunctionExpression(resultType, inputs...));
		expr->mFunctionName = functionName;
		return expr;
	}

	const std::string& functionName() const { return mFunctionName; }

private:

	using ExpressionBase::ExpressionBase;

	std::string mFunctionName;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public ExpressionBase
{
public:

	template<typename ...Expressions>
	static std::shared_ptr<ConstructorExpression> create(ValueType resultType, Expressions... inputs);

private:

	using ExpressionBase::ExpressionBase;
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

	static std::shared_ptr<SwizzleExpression> create(ValueType resultType, Swizzle swizzle, Expression input);

	Swizzle swizzle() const { return mSwizzle; }

private:
	using ExpressionBase::ExpressionBase;

	Swizzle mSwizzle{ Swizzle::X };
};


template<typename ...Expressions>
std::shared_ptr<ConstructorExpression> ConstructorExpression::create(ValueType resultType, Expressions... inputs)
{
	return std::shared_ptr<ConstructorExpression>(new ConstructorExpression(resultType, inputs...));
}

namespace DefaultSemantics
{
	/// The vertex shader output attributed with the POSITION semantic is interpreted as the vertex's final 3D position.
	constexpr auto Position      = "Position";
	constexpr auto Normal        = "Normal";
	constexpr auto Tangent       = "Tangent";
	constexpr auto Texcoord0     = "Texcoord0";
	constexpr auto WorldPosition = "WorldPosition";
	constexpr auto WorldNormal   = "WorldNormal";

	/// The fragment shader output attributed with the DEPTH semantic is interpreted as the pixel's final depth value.
	constexpr auto Depth = "Depth";

	// Default semantics for various commonly used uniforms
	constexpr auto ModelViewProjectionMatrix = "modelViewProjectionMatrix";
	constexpr auto ModelMatrix               = "modelMatrix";
	constexpr auto ProjectionMatrix          = "projectionMatrix";
	constexpr auto NormalMatrix              = "normalMatrix";
	constexpr auto ViewProjectionMatrix      = "viewProjectionMatrix";

} // namespace DefaultSemantics


///
class ShaderModule
{
public:

	explicit ShaderModule(ShaderStage shaderStage)
		: mShaderStage(shaderStage)
	{}

	void addOutput(std::string_view attributeSemantic, Expression expression);

	std::vector<AttributeExpressionPtr> inputs() const;

	std::vector<ParameterExpressionPtr> parameters() const;

	std::vector<AttributeExpressionPtr> outputs() const;

	std::vector<Expression> buildExpressionList() const;

	Coral::ShaderStage shaderStage() const { return mShaderStage; }

private:

	Coral::ShaderStage mShaderStage{ Coral::ShaderStage::VERTEX };
	std::vector<std::pair<std::string, AttributeExpressionPtr>> mOutputs;
};


class Program
{
public:
	void addVertexShaderOutput(std::string_view name, Expression expression);

	void addFragmentShaderOutput(std::string_view name, Expression expression);

	const ShaderModule* vertexShader() const;

	const ShaderModule* fragmentShader() const;

private:

	std::optional<ShaderModule> mVertexShader;
	std::optional<ShaderModule> mFragmentShader;
};

} // namespace Coral::ShaderGraph

#endif // !CORAL_SHADERGRAPH_HPP
