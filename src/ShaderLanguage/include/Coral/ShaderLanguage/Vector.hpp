#ifndef CORAL_SHADERLANGUAGE_VECTOR_HPP
#define CORAL_SHADERLANGUAGE_VECTOR_HPP

#include <Coral/ShaderLanguage/Scalar.hpp>

namespace Coral::ShaderLanguage
{

template<typename T1, typename T2>
ExpressionPtr makeScalarExpression(T2 arg)
{
	if constexpr (std::is_same_v<T2, T1>)
	{
		return Scalar<T1>(arg).source();
	}
	else if constexpr (std::is_same_v<T2, Scalar<T1>>)
	{
		return arg.source();
	}
	else
	{
		static_assert(false, "Invalid scalar value");
	}
}


/// Class representing a vector in the shader graph
template<typename T, size_t S> requires (S > 1)
struct Vector : public Variable
{

public:
	using Variable::Variable;

	static constexpr ValueType toShaderTypeId();

	/// Create a new Vector from scalars
	template<typename ...Ts> requires (sizeof...(Ts) == S)
	Vector(Ts... args)
		: Variable(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toShaderTypeId(), makeScalarExpression<T>(std::forward<Ts>(args))...))
	{
	}

	/// Create a new Vector from a smaller vector with the scalar values appended at the end
	template<size_t S2, typename ...Ts> requires (sizeof...(Ts) + S2 == S)
	Vector(const Vector<T, S2>& v, Ts... args)
		: Variable(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toShaderTypeId(), v.source(), makeScalarExpression<T>(std::forward<Ts>(args))...))
	{
	}

	Vector(const Vector& other)
		: Variable(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toShaderTypeId(), other.source()))
	{
	}

	Vector(Vector&& other)
		: Variable(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toShaderTypeId(), other.source()))
	{
	}

	Vector<T, S>& operator=(const Vector<T, S>& other)
	{
		mSource = ShaderModule::current()->addExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), other.source());
		return *this;
	}

	Vector<T, S>& operator=(Vector<T, S>&& other)
	{
		mSource = ShaderModule::current()->addExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), other.source());
		return *this;
	}

	/// Get the x component of the vector
	Scalar<T> x() const
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toShaderTypeId(), Swizzle::X, source()) };
	}

	/// Get the y component of the vector
	Scalar<T> y() const
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toShaderTypeId(), Swizzle::Y, source()) };
	}

	/// Get the z component of the vector
	Scalar<T> z() const requires(S >= 3)
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toShaderTypeId(), Swizzle::Z, source()) };
	}

	/// Get the w component of the vector
	Scalar<T> w() const requires(S >= 4)
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toShaderTypeId(), Swizzle::W, source()) };
	}

	/// Get the xy components of the vector
	Vector<T, 2> xy() const
	{
		return { std::make_shared<SwizzleExpression>(Vector<T, 2>::toShaderTypeId(), Swizzle::XY, source()) };
	}

	/// Get the xyz components of the vector
	Vector<T, 3> xyz() const requires(S >= 3)
	{
		return { std::make_shared<SwizzleExpression>(Vector<T, 3>::toShaderTypeId(), Swizzle::XYZ, source()) };
	}
};


template<> constexpr ValueType Vector<float, 2>::toShaderTypeId() { return ValueType::FLOAT2; }
template<> constexpr ValueType Vector<float, 3>::toShaderTypeId() { return ValueType::FLOAT3; }
template<> constexpr ValueType Vector<float, 4>::toShaderTypeId() { return ValueType::FLOAT4; }

template<> constexpr ValueType Vector<int, 2>::toShaderTypeId() { return ValueType::INT2; }
template<> constexpr ValueType Vector<int, 3>::toShaderTypeId() { return ValueType::INT3; }
template<> constexpr ValueType Vector<int, 4>::toShaderTypeId() { return ValueType::INT4; }


template<typename T, size_t S>
inline Vector<T, S> operator*(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::MULTIPLY, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::DIVIDE, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::ADD, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::SUBTRACT, rhs.source()) };
}

template<typename T, size_t S>
inline Vector<T, S> operator*(Vector<T, S> lhs, T rhs) { return lhs * Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, T rhs) { return lhs / Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, T rhs) { return lhs + Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, T rhs) { return lhs - Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator*(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::MULTIPLY, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::DIVIDE, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::ADD, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::SUBTRACT, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator*(T lhs, Vector<T, S> rhs) { return Scalar(lhs) * rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator/(T lhs, Vector<T, S> rhs) { return Scalar(lhs) / rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator+(T lhs, Vector<T, S> rhs) { return Scalar(lhs) + rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator-(T lhs, Vector<T, S> rhs) { return Scalar(lhs) - rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator*(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::MULTIPLY, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::DIVIDE, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::ADD, rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { createExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::SUBTRACT, rhs.source()) };
}

using float2 = Vector<float, 2>;
using float3 = Vector<float, 3>;
using float4 = Vector<float, 4>;

using int2 = Vector<int, 2>;
using int3 = Vector<int, 3>;
using int4 = Vector<int, 4>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_VECTOR_HPP
