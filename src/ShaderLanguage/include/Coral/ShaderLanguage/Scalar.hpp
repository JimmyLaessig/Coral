#ifndef CORAL_SHADERLANGUAGE_SCALAR_HPP
#define CORAL_SHADERLANGUAGE_SCALAR_HPP

#include <Coral/ShaderLanguage/Variable.hpp>
#include <Coral/ShaderLanguage/ShaderModule.hpp>
#include <Coral/ShaderLanguage/Attribute.hpp>

#include <concepts>

namespace Coral::ShaderLanguage
{

/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar : Variable
{
	using Variable::Variable;

	static constexpr ValueType toShaderTypeId();

	/// Create a new scalar from a constant
	Scalar(T v)
		: Variable(ShaderModule::current()->addExpression<ConstantExpression<T>>(v))
	{
	}

	template<typename U> requires (!std::is_same_v<T, U>)
	explicit operator Scalar<U>() const
	{
		return { ShaderModule::current()->addExpression<CastExpression>(Scalar<U>::toShaderTypeId(), source()) };
	}

	Scalar<T>& operator=(const Scalar<T>& other)
	{
		mSource = ShaderModule::current()->addExpression<OperatorExpression>(Scalar<T>::toShaderTypeId(), other.source());
		return *this;
	}

	Scalar<T>& operator=(Scalar<T>&& other)
	{
		mSource = ShaderModule::current()->addExpression<OperatorExpression>(Scalar<T>::toShaderTypeId(), other.source());
		return *this;
	}
};


template<> constexpr ValueType Scalar<float>::toShaderTypeId() { return ValueType::FLOAT; }
template<> constexpr ValueType Scalar<int>::toShaderTypeId() { return ValueType::INT; }
template<> constexpr ValueType Scalar<bool>::toShaderTypeId() { return ValueType::BOOL; }

template<typename T>
inline Scalar<T> operator*(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(Scalar<T>::toShaderTypeId(),  lhs.source(), Operator::MULTIPLY, rhs.source()) };
}


template<typename T>
inline Scalar<T> operator/(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(Scalar<T>::toShaderTypeId(), lhs.source(), Operator::DIVIDE, rhs.source()) };
}


template<typename T>
inline Scalar<T> operator+(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(Scalar<T>::toShaderTypeId(), lhs.source(), Operator::ADD, rhs.source()) };
}


template<typename T>
inline Scalar<T> operator-(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(Scalar<T>::toShaderTypeId(),  lhs.source(), Operator::SUBTRACT,rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator==(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(ValueType::BOOL, lhs.source(), Operator::EQUAL, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator!=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(ValueType::BOOL, lhs.source(), Operator::NOT_EQUAL, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator>(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(ValueType::BOOL, lhs.source(), Operator::GREATER, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator<(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(ValueType::BOOL, lhs.source(), Operator::LESS, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator>=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(ValueType::BOOL, lhs.source(), Operator::GREATER_OR_EQUAL, rhs.source()) };
}


template<typename T>
inline Scalar<bool> operator<=(Scalar<T> lhs, Scalar<T> rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(ValueType::BOOL,  lhs.source(), Operator::LESS_OR_EQUAL, rhs.source()) };
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
