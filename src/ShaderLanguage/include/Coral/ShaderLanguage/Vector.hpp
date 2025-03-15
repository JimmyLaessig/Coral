#ifndef CORAL_SHADERLANGUAGE_VECTOR_HPP
#define CORAL_SHADERLANGUAGE_VECTOR_HPP

#include <Coral/ShaderLanguage/Scalar.hpp>

namespace Coral::ShaderLanguage
{

template<typename T1, typename T2>
ShaderGraph::NodePtr makeScalarExpression(T2 arg)
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
	struct Vector : public ValueTypeBase
{

public:
	using ValueTypeBase::ValueTypeBase;

	static constexpr ShaderGraph::ValueType toShaderTypeId();

	/// Create a new Vector from scalars
	template<typename ...Ts> requires (sizeof...(Ts) == S)
		Vector(Ts... args)
		: ValueTypeBase(ShaderGraph::Node::createConstructor(Vector<T, S>::toShaderTypeId(), makeScalarExpression<T>(std::forward<Ts>(args))...))
	{
	}

	/// Create a new Vector from a smaller vector with the scalar values appended at the end
	template<size_t S2, typename ...Ts> requires (sizeof...(Ts) + S2 == S)
		Vector(const Vector<T, S2>& v, Ts... args)
		: ValueTypeBase(ShaderGraph::Node::createConstructor(Vector<T, S>::toShaderTypeId(), v.source(), makeScalarExpression<T>(std::forward<Ts>(args))...))
	{
	}

	/// Get the x component of the vector
	Scalar<T> x() const
	{
		return ShaderGraph::Node::createSwizzle(Scalar<T>::toShaderTypeId(), ShaderGraph::Swizzle::X, source());
	}

	/// Get the y component of the vector
	Scalar<T> y() const
	{
		return ShaderGraph::Node::createSwizzle(Scalar<T>::toShaderTypeId(), ShaderGraph::Swizzle::Y, source());
	}

	/// Get the z component of the vector
	Scalar<T> z() const requires(S >= 3)
	{
		return ShaderGraph::Node::createSwizzle(Scalar<T>::toShaderTypeId(), ShaderGraph::Swizzle::Z, source());
	}

	/// Get the w component of the vector
	Scalar<T> w() const requires(S >= 4)
	{
		return ShaderGraph::Node::createSwizzle(Scalar<T>::toShaderTypeId(), ShaderGraph::Swizzle::W, source());
	}

	/// Get the xy components of the vector
	Vector<T, 2> xy() const
	{
		return ShaderGraph::Node::createSwizzle(Scalar<T>::toShaderTypeId(), ShaderGraph::Swizzle::XY, source());
	}

	/// Get the xyw components of the vector
	Vector<T, 3> xyz() const requires(S >= 3)
	{
		return ShaderGraph::Node::createSwizzle(Scalar<T>::toShaderTypeId(), ShaderGraph::Swizzle::XYZ, source());
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
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, Scalar<T> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::SUBTRACT, lhs.source(), rhs.source()) };
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
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Scalar<T> lhs, Vector<T, S> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::SUBTRACT, lhs.source(), rhs.source()) };
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
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, Vector<T, S> rhs)
{
	return { ShaderGraph::Node::CreateOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::SUBTRACT, lhs.source(), rhs.source()) };
}

using Float2 = Vector<float, 2>;
using Float3 = Vector<float, 3>;
using Float4 = Vector<float, 4>;

using Int2 = Vector<int, 2>;
using Int3 = Vector<int, 3>;
using Int4 = Vector<int, 4>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_VECTOR_HPP
