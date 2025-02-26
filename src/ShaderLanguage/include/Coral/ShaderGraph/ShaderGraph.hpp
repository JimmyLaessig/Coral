#ifndef CORAL_SHADERGRAPH_SHADERGRAPH_HPP
#define CORAL_SHADERGRAPH_SHADERGRAPH_HPP

#include <array>
#include <concepts>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <variant>
#include <vector>
#include <set>
#include <map>

#include <Coral/Coral.hpp>
namespace Coral::ShaderLanguage::ShaderGraph
{

enum class ShaderTypeId
{
	/// 32-bit integer
	INT,
	/// 2-component 32-bit integer vector
	INT2,
	/// 3-component 32-bit integer vector
	INT3,
	/// 4-component 32-bit integer vector
	INT4,
	/// 32-bit floating-point type
	FLOAT,
	/// 2-component 32-bit floating-point vector
	FLOAT2,
	/// 3-component 32-bit floating-point vector
	FLOAT3,
	/// 4-component 32-bit floating-point vector
	FLOAT4,
	/// 3x3 32-bit floating-point matrix
	FLOAT3X3,
	/// 4x4 32-bit floating-point matrix
	FLOAT4X4,
	/// 2D texture
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


using Expression = std::variant<
	ConstantPtr<float>,
	ConstantPtr<int>,
	AttributeExpressionPtr,
	ParameterExpressionPtr,
	OperatorExpressionPtr,
	NativeFunctionExpressionPtr,
	ConstructorExpressionPtr,
	CastExpressionPtr,
	SwizzleExpressionPtr>;


class ExpressionBase
{
public:

	/// Get the ShaderTypeId of the expression's output
	ShaderTypeId outputShaderTypeId() const;

	/// Get the ShaderTypeIds of the expression's inputs
	std::vector<ShaderTypeId> inputTypeIds() const;

	/// Get the inputs of the expression
	std::span<const Expression> inputs() const;

protected:

	template<typename ...Expressions>
	ExpressionBase(ShaderTypeId outputTypeId, Expressions... inputs)
		: mOutputTypeId(outputTypeId)
	{
		(mInputs.push_back(inputs), ...);
	}

	void addInput(Expression expr)
	{
		mInputs.push_back(expr);
	}

private:

	ShaderTypeId mOutputTypeId;

	std::vector<Expression> mInputs;
};


/// Defines a constant scalar value expression
template<ScalarTypes Scalar>
class Constant : public ShaderGraph::ExpressionBase
{
public:
	static std::shared_ptr<Constant> create(Scalar value)
	{
		ShaderTypeId id;
		if constexpr (std::is_same_v<Scalar, float>)
		{
			id = ShaderTypeId::FLOAT;
		}
		else if constexpr (std::is_same_v<Scalar, int>)
		{
			id = ShaderTypeId::INT;
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

	using ShaderGraph::ExpressionBase::ExpressionBase;

	Scalar mValue{ 0 };
};


/// Defines a shader attribute value
class AttributeExpression : public ShaderGraph::ExpressionBase
{
public:

	// Create a new Vertex shader input attribute
	static inline std::shared_ptr<AttributeExpression> create(ShaderTypeId resultType, std::string_view name)
	{
		std::shared_ptr<AttributeExpression> expr(new AttributeExpression(resultType));
		expr->mName = std::string(name.begin(), name.end());
		return expr;
	}

	// Create a new shader output attribute
	static inline std::shared_ptr<AttributeExpression> create(Expression source, std::string_view name)
	{
		auto resultType = std::visit([](auto expr) { return expr->outputShaderTypeId(); }, source);
		std::shared_ptr<AttributeExpression> expr(new AttributeExpression(resultType, source));
		expr->mName = std::string(name.begin(), name.end());
		return expr;
	}

	const std::string& name() const { return mName; }

private:

	using ShaderGraph::ExpressionBase::ExpressionBase;

	std::string mName;
};


/// Defines a mutable shader parameter value
class ParameterExpression : public ShaderGraph::ExpressionBase
{
public:

	static inline std::shared_ptr<ParameterExpression> create(ShaderTypeId resultType, std::string_view name)
	{
		std::shared_ptr<ParameterExpression> expr(new ParameterExpression(resultType));
		expr->mName = std::string(name.begin(), name.end());
		return expr;
	}

	const std::string& name() const { return mName; }

private:

	using ShaderGraph::ExpressionBase::ExpressionBase;

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
class OperatorExpression : public ShaderGraph::ExpressionBase
{
public:

	using ShaderGraph::ExpressionBase::ExpressionBase;

	static inline std::shared_ptr<OperatorExpression> create(ShaderTypeId resultType, Operator op, Expression lhs, Expression rhs)
	{
		std::shared_ptr<OperatorExpression> expr(new OperatorExpression(resultType, lhs, rhs));
		expr->mOperator = op;
		return expr;
	}

	Operator getOperator() const { return mOperator; }

private:

	Operator mOperator{ Operator::ADD };
};


/// Class defining a cast operator expression
class CastExpression : public ShaderGraph::ExpressionBase
{
public:

	static inline std::shared_ptr<CastExpression> create(ShaderTypeId resultType, Expression input)
	{
		return std::shared_ptr<CastExpression>(new CastExpression(resultType, input ));
	}

private:

	using ShaderGraph::ExpressionBase::ExpressionBase;

};


/// Class defining a native function call in the shader graph
class NativeFunctionExpression : public ShaderGraph::ExpressionBase
{
public:

	template<typename ...Expressions>
	static inline std::shared_ptr<NativeFunctionExpression> create(ShaderTypeId resultType, 
																   std::string_view functionName, 
																   Expressions ...inputs)
	{
		std::shared_ptr<NativeFunctionExpression> expr(new NativeFunctionExpression(resultType, inputs...));
		expr->mFunctionName = functionName;
		return expr;
	}

	const std::string& functionName() const { return mFunctionName; }

private:

	using ShaderGraph::ExpressionBase::ExpressionBase;

	std::string mFunctionName;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public ShaderGraph::ExpressionBase
{
public:

	template<typename ...Expressions>
	static inline std::shared_ptr<ConstructorExpression> create(ShaderTypeId resultType, Expressions... inputs)
	{
		auto expr = std::shared_ptr<ConstructorExpression>(new ConstructorExpression(resultType, inputs...));

		return expr;
	}

private:

	using ShaderGraph::ExpressionBase::ExpressionBase;
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


class SwizzleExpression : public ShaderGraph::ExpressionBase
{
public:

	static inline std::shared_ptr<SwizzleExpression> create(ShaderTypeId resultType, Swizzle swizzle, Expression input)
	{
		std::shared_ptr<SwizzleExpression> expr(new SwizzleExpression(resultType, input));
		expr->mSwizzle = swizzle;
		return expr;

	}

	Swizzle swizzle() const { return mSwizzle; }

private:
	using ShaderGraph::ExpressionBase::ExpressionBase;

	Swizzle mSwizzle{ Swizzle::X };
};


enum class ShaderStage
{
	VERTEX = 0,
	FRAGMENT = 1,
};


namespace DefaultSemantics
{
	/// The vertex shader output attributed with the POSITION semantic is interpreted as the vertex's final 3D position.
	constexpr auto POSITION  = "Position";
	constexpr auto NORMAL    = "Normal";
	constexpr auto TANGENT   = "Tangent";
	constexpr auto TEXCOORD0 = "Texcoord0";

	/// The fragment shader output attributed with the DEPTH semantic is interpreted as the pixel's final depth value.
	constexpr auto DEPTH     = "Depth";
} // namespace DefaultSemantics

///
class ShaderModule
{
public:

	ShaderModule() = default;

	explicit ShaderModule(ShaderStage shaderStage)
		: mShaderStage(shaderStage)
	{}

	void addOutput(std::string_view attributeSemantic, Expression expression);

	std::vector<AttributeExpressionPtr> inputs() const;

	std::vector<ParameterExpressionPtr> parameters() const;

	std::vector<AttributeExpressionPtr> outputs() const;

	std::vector<Expression> buildExpressionList() const;

	ShaderStage shaderStage() const { return mShaderStage; }
private:

	ShaderStage mShaderStage{ ShaderStage::VERTEX };
	std::vector<std::pair<std::string, AttributeExpressionPtr>> mOutputs;
};


///
class ShaderProgram
{
public:

	void addOutput(ShaderStage shaderStage, std::string_view name, Expression expression);

	const ShaderModule* shaderModule(ShaderStage stage) const;

private:

	std::map<ShaderStage, ShaderModule> mShaderModules;
};

} // namespace Coral::Slang::ShaderGraph

#endif // !CORAL_SHADERGRAPH_SHADERGRAPH_HPP
