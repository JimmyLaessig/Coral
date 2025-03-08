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


class Node;
using NodePtr = std::shared_ptr<Node>;


/// Base class for a shader graph expression
class ExpressionBase
{
public:

	/// Get the ShaderTypeId of the expression's output
	ValueType outputValueType() const;

	/// Get the ShaderTypeIds of the expression's inputs
	//std::vector<ValueType> inputValueTypes() const;

	///// Get the inputs of the expression
	//std::span<Expression*> inputs() const;

	NodePtr node() const;

	const std::vector<NodePtr>& inputs() const;

protected:

	ExpressionBase(ValueType outputValueTypeId, NodePtr node);

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

	Constant(float value, NodePtr node)
		: ExpressionBase(ValueType::FLOAT, node)
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

	Constant(int value, NodePtr node)
		: ExpressionBase(ValueType::INT, node)
		, mValue(value)
	{
		mValue = value;
	}

	int value() const { return mValue; }

private:

	int mValue{ 0 };
};


/// Defines a shader attribute value
class AttributeExpression : public ExpressionBase
{
public:

	// Create a new shader input attribute
	AttributeExpression(ValueType outputType, std::string_view name, NodePtr node);

	// Create a new shader output attribute
	AttributeExpression(std::string_view name, NodePtr node);

	const std::string& name() const;

private:

	std::string mName;
};


/// Defines a shader parameter value (also known as uniform)
class ParameterExpression : public ExpressionBase
{
public:

	// Create a new shader input attribute
	ParameterExpression(ValueType outputType, std::string_view name, NodePtr node);

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
	SUBTRACT
};


/// Class defining an arithmetic operator expression
class OperatorExpression : public ExpressionBase
{
public:

	OperatorExpression(ValueType outputType, Operator op, NodePtr node);

	Operator getOperator() const;

private:
	
	Operator mOperator{ Operator::ADD };
};


/// Class defining a cast operator expression
class CastExpression : public ExpressionBase
{
public:

	CastExpression(ValueType outputValueType, NodePtr node);

};


/// Class defining a native function call in the shader graph
class NativeFunctionExpression : public ExpressionBase
{
public:

	NativeFunctionExpression(ValueType outputValueType, std::string_view name, NodePtr node);

	const std::string& functionName() const;

private:

	std::string mFunctionName;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public ExpressionBase
{
public:

	ConstructorExpression(ValueType outputValueType, NodePtr node);

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

	SwizzleExpression(ValueType resultType, Swizzle swizzle, NodePtr node);

	Swizzle swizzle() const;

private:

	Swizzle mSwizzle{ Swizzle::X };
};


using Expression = std::variant<Constant<float>,
							    Constant<int>,
							    AttributeExpression,
							    ParameterExpression,
							    OperatorExpression,
							    NativeFunctionExpression,
							    ConstructorExpression,
							    CastExpression,
							    SwizzleExpression>;


inline ValueType
getOutputValueType(const Expression& expr)
{
	return std::visit([](const auto& ex) { return ex.outputValueType(); }, expr);
}


inline ExpressionBase&
cast(Expression& expr)
{
	return std::visit([](auto& ex) -> ExpressionBase& { return ex; }, expr);
}


inline const ExpressionBase&
cast(const Expression& expr)
{
	return std::visit([](const auto& ex) -> const ExpressionBase& { return ex; }, expr);
}


inline NodePtr
getNode(Expression& expr)
{
	return std::visit([](const auto& ex) -> NodePtr { return ex.node(); }, expr);
}

/// 
class Node : public std::enable_shared_from_this<Node>
{
public:

	template<typename ...Nodes>
	Node(Nodes... inputs)
	{
		(mInputs.push_back(inputs), ...);
	}

	/// Get the expression of the node
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

	static NodePtr createAttribute(NodePtr input, std::string_view name);

	static NodePtr createParameter(ValueType valueType, std::string_view name);

	static NodePtr createOperator(ValueType valueType, Operator op, NodePtr lhs, NodePtr rhs);

	static NodePtr createCast(ValueType valueType, NodePtr input);

	template<typename ...NodePtrs>
	static NodePtr createNativeFunction(ValueType valueType, std::string_view name, NodePtrs... inputs)
	{
		auto node = std::make_shared<Node>(inputs...);
		node->mExpression = NativeFunctionExpression(valueType, name, node);
		return node;
	}

	template<typename ...NodePtrs>
	static NodePtr createConstructor(ValueType valueType, NodePtrs... inputs)
	{
		auto node = std::make_shared<Node>(inputs...);
		node->mExpression = ConstructorExpression(valueType, node);
		return node;
	}

	static NodePtr createSwizzle(ValueType valueType, Swizzle swizzle, NodePtr input);

private:

	Expression mExpression = Constant<float>(0, nullptr);

	std::vector<NodePtr> mInputs;
};


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

	void addOutput(std::string_view attributeSemantic, NodePtr node);

	std::vector<const AttributeExpression*> inputs() const;

	std::vector<const ParameterExpression*> parameters() const;

	std::vector<const AttributeExpression*> outputs() const;

	std::vector<const Expression*> buildExpressionList() const;

	Coral::ShaderStage shaderStage() const { return mShaderStage; }

private:

	Coral::ShaderStage mShaderStage{ Coral::ShaderStage::VERTEX };
	std::vector<std::pair<std::string, std::shared_ptr<Node>>> mOutputs;
};


class Program
{
public:
	void addVertexShaderOutput(std::string_view name, NodePtr node);

	void addFragmentShaderOutput(std::string_view name, NodePtr node);

	const ShaderModule* vertexShader() const;

	const ShaderModule* fragmentShader() const;

private:

	std::optional<ShaderModule> mVertexShader;
	std::optional<ShaderModule> mFragmentShader;
};

} // namespace Coral::ShaderGraph

#endif // !CORAL_SHADERGRAPH_HPP
