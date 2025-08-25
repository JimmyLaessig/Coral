#ifndef CORAL_SHADERLANGUAGE_SCALAR_HPP
#define CORAL_SHADERLANGUAGE_SCALAR_HPP

#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <concepts>

namespace Coral::ShaderLanguage
{

using ValueType = ShaderGraph::ValueType;


/// Base struct for all return values of shader graph expressions
/**
 * Objects that derive from this type act as stack-allocated wrappers arround a shader graph operation and define the
 * output type of said operation. Objects of this type are widely used as inputs to functions that build up new shader
 * graph operations. For example, the ´Float´ struct is a proxy for a floating-point value that is created by a shader
 * graph operation. The object itself does not hold the actual value but stores a pointer to the shader graph node from
 * which it was calculated. The user is only required to interact with the stack-allocated ShaderGraphResult objects
 * and function calls to build the shader graph. The creation and storage of the actual ShaderGraph is abstracted away
 * by this level of indirection.
 */
struct Value
{
public:

	Value(ShaderGraph::ExpressionPtr source)
		: mSource(source)
	{
	}

	// Get the wrapped ShaderGraph node
	ShaderGraph::ExpressionPtr source() const
	{
		return mSource;
	}

	/// Get the type id of the value
	ValueType typeId() const
	{
		return mSource->outputValueType();
	}

private:

	ShaderGraph::ExpressionPtr mSource;
};


/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar : Value
{
	using Value::Value;

	static constexpr ValueType toShaderTypeId();

	/// Create a new scalar from a constant
	Scalar(T v)
		: Value(std::make_shared<ShaderGraph::ConstantExpression<T>>(v))
	{
	}

	template<typename U> requires (!std::is_same_v<T, U>)
		explicit operator Scalar<U>() const
	{
		return { std::make_shared<ShaderGraph::CastExpression>(Scalar<U>::toShaderTypeId(), source()) };
	}
};


template<> constexpr ValueType Scalar<float>::toShaderTypeId() { return ValueType::FLOAT; }
template<> constexpr ValueType Scalar<int>::toShaderTypeId() { return ValueType::INT; }
template<> constexpr ValueType Scalar<bool>::toShaderTypeId() { return ValueType::BOOL; }

template<typename T>
inline Scalar<T> operator*(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(Scalar<T>::toShaderTypeId(),  lhs.source(), ShaderGraph::Operator::MULTIPLY, rhs.source()) };
}


template<typename T>
inline Scalar<T> operator/(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(Scalar<T>::toShaderTypeId(), lhs.source(), ShaderGraph::Operator::DIVIDE, rhs.source()) };
}


template<typename T>
inline Scalar<T> operator+(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(Scalar<T>::toShaderTypeId(), lhs.source(), ShaderGraph::Operator::ADD, rhs.source()) };
}


template<typename T>
inline Scalar<T> operator-(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(Scalar<T>::toShaderTypeId(),  lhs.source(), ShaderGraph::Operator::SUBTRACT,rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator==(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(ValueType::BOOL, lhs.source(), ShaderGraph::Operator::EQUAL, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator!=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(ValueType::BOOL, lhs.source(), ShaderGraph::Operator::NOT_EQUAL, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator>(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(ValueType::BOOL, lhs.source(), ShaderGraph::Operator::GREATER, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator<(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(ValueType::BOOL, lhs.source(), ShaderGraph::Operator::LESS, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator>=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(ValueType::BOOL, lhs.source(), ShaderGraph::Operator::GREATER_OR_EQUAL, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator<=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { std::make_shared<ShaderGraph::OperatorExpression>(ValueType::BOOL,  lhs.source(), ShaderGraph::Operator::LESS_OR_EQUAL, rhs.source()) };
}

template<typename T>
inline Scalar<bool> operator*(Scalar<T> lhs, T rhs) { return lhs * Scalar(rhs); }


template<typename T>
inline Scalar<T> operator/(Scalar<T> lhs, T rhs) { return lhs / Scalar(rhs); }


template<typename T>
inline Scalar<T> operator+(Scalar<T> lhs, T rhs) { return lhs + Scalar(rhs); }


template<typename T>
inline Scalar<T> operator-(Scalar<T> lhs, T rhs) { return lhs - Scalar(rhs); }


template<typename T>
inline Scalar<T> operator*(T lhs, Scalar<T> rhs) { return Scalar(lhs) * rhs; }


template<typename T>
inline Scalar<T> operator/(T lhs, Scalar<T> rhs) { return Scalar(lhs) / rhs; }


template<typename T>
inline Scalar<T> operator+(T lhs, Scalar<T> rhs) { return Scalar(lhs) + rhs; }


template<typename T>
inline Scalar<T> operator-(T lhs, Scalar<T> rhs) { return Scalar(lhs) - rhs; }


template<typename T>
inline Scalar<bool> operator==(Scalar<T> lhs, T rhs) { return lhs == Scalar(rhs); }


template<typename T>
inline Scalar<bool> operator!=(Scalar<T> lhs, T rhs) { return lhs != Scalar(rhs); }


template<typename T>
inline Scalar<bool> operator>(Scalar<T> lhs, T rhs) { return lhs > Scalar(rhs); }


template<typename T>
inline Scalar<bool> operator<(Scalar<T> lhs, T rhs) { return lhs < Scalar(rhs); }


template<typename T>
inline Scalar<bool> operator>=(Scalar<T> lhs, T rhs) { return lhs >= Scalar(rhs); }


template<typename T>
inline Scalar<bool> operator<=(Scalar<T> lhs, T rhs) { return lhs <= Scalar(rhs); }


template<typename T>
inline Scalar<bool> operator==(T lhs, Scalar<T> rhs) { return Scalar(lhs) == rhs; }


template<typename T>
inline Scalar<bool> operator!=(T lhs, Scalar<T> rhs) { return Scalar(lhs) != rhs; }


template<typename T>
inline Scalar<bool> operator>(T lhs, Scalar<T> rhs) { return Scalar(lhs) > rhs; }


template<typename T>
inline Scalar<bool> operator<(T lhs, Scalar<T> rhs) { return Scalar(lhs) < rhs; }


template<typename T>
inline Scalar<bool> operator>=(T lhs, Scalar<T> rhs) { return Scalar(lhs) >= rhs; }


template<typename T>
inline Scalar<bool> operator<=(T lhs, Scalar<T> rhs) { return Scalar(lhs) <= rhs; }


using Float = Scalar<float>;
using Int   = Scalar<int>;
using Bool  = Scalar<bool>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SCALAR_HPP
