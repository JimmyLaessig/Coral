#ifndef CORAL_SHADERLANGUAGE_EXPRESSIONS_HPP
#define CORAL_SHADERLANGUAGE_EXPRESSIONS_HPP

#include <Coral/ShaderLanguage/Value.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Coral::ShaderLanguage
{

class Node;
using NodePtr = std::shared_ptr<Node>;
using ConstNodePtr = std::shared_ptr<const Node>;

class ExpressionBase
{
public:
	ExpressionBase(ValueType type)
		: mType(type)
	{
	}

	ValueType valueType() const
	{
		return mType;
	}

	NodePtr node()
	{
		return mNode.lock();
	}

	ConstNodePtr node() const
	{
		return mNode.lock();
	}

	virtual void setNode(NodePtr node)
	{
		mNode = node;
	}

private:

	std::weak_ptr<Node> mNode;

	ValueType mType{};
};


/// Class defining a constant scalar value expression
template<typename Scalar>
class ConstantExpression : public ExpressionBase
{
public:

	ConstantExpression(Scalar value) requires std::same_as<Scalar, int>
		: ExpressionBase(ValueType::INT)
		, mValue(value)
	{
	}

	ConstantExpression(Scalar value) requires std::same_as<Scalar, float>
		: ExpressionBase(ValueType::FLOAT)
		, mValue(value)
	{
	}

	ConstantExpression(Scalar value) requires std::same_as<Scalar, bool>
		: ExpressionBase(ValueType::BOOL)
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
class InputAttributeExpression : public ExpressionBase
{
public:

	InputAttributeExpression(const ValueType& type, uint32_t location, std::string_view name)
		: ExpressionBase(type)
		, mLocation(location)
		, mName(name)
	{
	}

	const std::string& name() const { return mName; }

	uint32_t location() const { return mLocation; }

	void setNode(NodePtr node) override
	{
		ExpressionBase::setNode(node);
		//node->setInlineIfPossible();
	}

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
class OutputAttributeExpression : public ExpressionBase
{
public:

	OutputAttributeExpression(ValueType valueType, uint32_t location, std::string_view name);

	OutputAttributeExpression(ValueType valueType, DefaultSemantics attribute);

	const AttributeBindingInfo& BindingInfo() const;

private:

	AttributeBindingInfo mBindingInfo;
};


class UniformBufferInfo
{
public:

	UniformBufferInfo(uint32_t location, std::string_view name)
		: mLocation(location)
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


class UniformExpression : public ExpressionBase
{
public:

	UniformExpression(const ValueType& type, std::string_view name, std::shared_ptr<UniformBufferInfo> buffer)
		: ExpressionBase(type)
		, mName(name)
		, mBufferInfo(buffer)
	{
	}

	void setNode(NodePtr node) override
	{
		ExpressionBase::setNode(node);
		//node->setInlineIfPossible();
	}

	std::shared_ptr<const UniformBufferInfo> bufferInfo() const
	{
		return mBufferInfo;
	}

	const std::string& name() const { return mName; }

private:

	std::string mName;

	std::shared_ptr< UniformBufferInfo> mBufferInfo;

};


class SamplerExpression : public ExpressionBase
{
public:

	SamplerExpression(uint32_t location, std::string_view name)
		: ExpressionBase(ValueType::SAMPLER2D)
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
class OperatorExpression : public ExpressionBase
{
public:

	OperatorExpression(ValueType outputType, Operator op)
		: ExpressionBase(outputType)
		, mOperator(op)
	{
	}

	Operator getOperator() const { return mOperator; }

private:

	Operator mOperator;
};


/// Class defining a cast operator expression
class CastExpression : public ExpressionBase
{
public:

	CastExpression(ValueType outputType)
		: ExpressionBase(outputType)
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
class NativeFunctionExpression : public ExpressionBase
{
public:

	NativeFunctionExpression(ValueType valueType, NativeFunction function)
		: ExpressionBase(valueType)
		, mFunction(function)
	{
	}

	NativeFunction Function() const { return mFunction; }

private:

	NativeFunction mFunction;
};


/// Class defining a native constructor call in the shader graph
class ConstructorExpression : public ExpressionBase
{
public:

	ConstructorExpression(ValueType valueType)
		: ExpressionBase(valueType)
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
class SwizzleExpression : public ExpressionBase
{
public:

	SwizzleExpression(ValueType outputType, Swizzle swizzle)
		: ExpressionBase(outputType)
		, mSwizzle(swizzle)
	{
	}

	Swizzle swizzle() const { return mSwizzle; };

private:

	Swizzle mSwizzle;
};


using Expression = std::variant<ConstantExpression<float>, 
	                            ConstantExpression<int>, 
	                            ConstantExpression<bool>,
	                            InputAttributeExpression,
	                            OutputAttributeExpression,
	                            UniformExpression,
	                            SamplerExpression,
	                            OperatorExpression,
	                            CastExpression,
	                            NativeFunctionExpression,
	                            ConstructorExpression,
	                            SwizzleExpression>;

} // namespace Coral::ShaderLanguage

#endif // !CORAL_SHADERLANGUAGE_EXPRESSIONS_HPP
