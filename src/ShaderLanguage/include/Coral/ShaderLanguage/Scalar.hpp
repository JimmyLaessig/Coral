#ifndef CORAL_SHADERLANGUAGE_SCALAR_HPP
#define CORAL_SHADERLANGUAGE_SCALAR_HPP


#include <Coral/ShaderGraph/ShaderGraph.hpp>

#include <concepts>


namespace Coral::ShaderLanguage
{

using ValueType = Coral::ShaderGraph::ValueType;


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
struct ValueTypeBase
{
public:

	ValueTypeBase(ShaderGraph::NodePtr source)
		: mSource(source)
	{
	}

	// Get the ShaderGraph node from which this value originates
	ShaderGraph::NodePtr source() const
	{
		return mSource;
	}

	/// Get the type id of the value
	ValueType typeId() const
	{
		return mSource->outputValueType();
	}

private:

	ShaderGraph::NodePtr mSource;
};


/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar : ValueTypeBase
{
	using ValueTypeBase::ValueTypeBase;

	static constexpr ValueType toShaderTypeId();

	/// Create a new scalar from a constant
	Scalar(T v)
		: ValueTypeBase(ShaderGraph::Node::createConstant(v))
	{
	}

	template<typename U> requires (!std::is_same_v<T, U>)
		explicit operator Scalar<U>() const
	{
		return { ShaderGraph::Node::createCast(Scalar<U>::toShaderTypeId(), source()) };
	}
};


template<> constexpr ValueType Scalar<float>::toShaderTypeId() { return ValueType::FLOAT; }
template<> constexpr ValueType Scalar<int>::toShaderTypeId() { return ValueType::INT; }
template<> constexpr ValueType Scalar<bool>::toShaderTypeId() { return ValueType::BOOL; }

template<typename T>
inline Scalar<T> operator*(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Scalar<T>::toShaderTypeId(), ShaderGraph::Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<T> operator/(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Scalar<T>::toShaderTypeId(), ShaderGraph::Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<T> operator+(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Scalar<T>::toShaderTypeId(), ShaderGraph::Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<T> operator-(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Scalar<T>::toShaderTypeId(), ShaderGraph::Operator::SUBTRACT, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator==(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(ValueType::BOOL, ShaderGraph::Operator::EQUAL, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator!=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(ValueType::BOOL, ShaderGraph::Operator::NOT_EQUAL, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator>(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(ValueType::BOOL, ShaderGraph::Operator::GREATER, lhs.source(), rhs.source()) };
}


template<typename T>
inline  Scalar<bool> operator<(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(ValueType::BOOL, ShaderGraph::Operator::LESS, lhs.source(), rhs.source()) };
}


template<typename T>
inline  Scalar<bool> operator>=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(ValueType::BOOL, ShaderGraph::Operator::GREATER_OR_EQUAL, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator<=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(ValueType::BOOL, ShaderGraph::Operator::LESS_OR_EQUAL, lhs.source(), rhs.source()) };
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
inline Scalar<bool> operator>(T lhs, Scalar<T> rhs) { return  Scalar(lhs) > rhs; }


template<typename T>
inline Scalar<bool> operator<(T lhs, Scalar<T> rhs) { return  Scalar(lhs) < rhs; }


template<typename T>
inline Scalar<bool> operator>=(T lhs, Scalar<T> rhs) { return Scalar(lhs) >= rhs; }


template<typename T>
inline Scalar<bool> operator<=(T lhs, Scalar<T> rhs) { return Scalar(lhs) <= rhs; }


using Float = Scalar<float>;
using Int = Scalar<int>;
using Bool = Scalar<bool>;


} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SCALAR_HPP
