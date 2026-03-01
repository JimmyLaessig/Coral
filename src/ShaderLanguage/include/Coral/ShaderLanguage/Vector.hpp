#ifndef CORAL_SHADERLANGUAGE_VECTOR_HPP
#define CORAL_SHADERLANGUAGE_VECTOR_HPP

#include <Coral/ShaderLanguage/Scalar.hpp>

namespace Coral::ShaderLanguage
{

template<typename T, size_t S> requires (S > 1)
struct Vector;

template<typename V, typename T, size_t S>
concept VectorType = std::same_as<std::remove_cvref_t<V>, Vector<T, S>>;

template<>
struct TypeTraits<Vector<float, 2>>
{
	constexpr static ValueType ValueType = ValueType::FLOAT2;
};

template<>
struct TypeTraits<Vector<float, 3>>
{
	constexpr static ValueType ValueType = ValueType::FLOAT3;
};

template<>
struct TypeTraits<Vector<float, 4>>
{
	constexpr static ValueType ValueType = ValueType::FLOAT4;
};

template<>
struct TypeTraits<Vector<int, 2>>
{
	constexpr static ValueType ValueType = ValueType::BOOL;
};

template<>
struct TypeTraits<Vector<int, 3>>
{
	constexpr static ValueType ValueType = ValueType::BOOL;
};

template<>
struct TypeTraits<Vector<int, 4>>
{
	constexpr static ValueType ValueType = ValueType::BOOL;
};


/// Class representing a vector in the shader graph
template<typename T, size_t S> requires (S > 1)
struct Vector : public Value
{
public:

	using Value::Value;

	static constexpr ValueType ValueType = TypeTraits<Vector<T, S>>::ValueType;

	Vector() requires (S == 1)
		: Vector(T(0))
	{
	}

	Vector() requires (S == 2)
		: Vector(T(0), T(0))
	{
	}

	Vector() requires (S == 3)
		: Vector(T(0), T(0), T(0))
	{
	}

	Vector() requires (S == 4)
		: Vector(T(0), T(0), T(0), T(0))
	{
	}

	/// Create a new Vector from scalars
	template<typename ...Ts> requires (sizeof...(Ts) == S)
	Vector(Ts&&... args)
		: Value(ShaderGraph::PushExpression<ConstructorExpression>(Vector<T, S>::ValueType, Scalar<T>(args).source()...))
	{
	}

	/// Create a new Vector from a smaller vector with the scalar values appended at the end
	template<size_t S2, typename ...Ts> requires (sizeof...(Ts) + S2 == S)
	Vector(const Vector<T, S2>& v, Ts... args)
		: Value(ShaderGraph::PushExpression<ConstructorExpression>(Vector<T, S>::ValueType, v.source(), Scalar<T>(args).source()...))
	{
	}

	Vector(const Vector& other)
		: Value(ShaderGraph::PushExpression<ConstructorExpression>(Vector<T, S>::ValueType, other.source()))
	{
	}

	Vector(Vector&& other)
		: Value(other.source())
	{
	}

	Vector<T, S>& operator=(const Vector<T, S>& other)
	{
		setSource(ShaderGraph::PushExpression<OperatorExpression>(Vector<T, S>::ValueType, source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	Vector<T, S>& operator=(Vector<T, S>&& other)
	{
		setSource(ShaderGraph::PushExpression<OperatorExpression>(Vector<T, S>::ValueType, source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	/// Get the x component of the vector
	Scalar<T> x() const
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::ValueType, Swizzle::X, source()) };
	}

	/// Get the y component of the vector
	Scalar<T> y() const
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::ValueType, Swizzle::Y, source()) };
	}

	/// Get the z component of the vector
	Scalar<T> z() const requires(S >= 3)
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::ValueType, Swizzle::Z, source()) };
	}

	/// Get the w component of the vector
	Scalar<T> w() const requires(S >= 4)
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::ValueType, Swizzle::W, source()) };
	}

	/// Get the xy components of the vector
	Vector<T, 2> xy() const
	{
		return { std::make_shared<SwizzleExpression>(Vector<T, 2>::ValueType, Swizzle::XY, source()) };
	}

	/// Get the xyz components of the vector
	Vector<T, 3> xyz() const requires(S >= 3)
	{
		return { std::make_shared<SwizzleExpression>(Vector<T, 3>::ValueType, Swizzle::XYZ, source()) };
	}

	/// Vector multiplication operator
	/**
	 * The following multiplications are supported:
	 * vector * vector
	 * vector * scalar
	 * scalar * vector
	 * vector * T
	 *      T * vector
	 */
	template<typename LHS, typename RHS>
	friend Vector<T, S> operator*(LHS&& lhs, RHS&& rhs)
		requires (VectorType<LHS, T, S> && VectorType<RHS, T, S>)  ||
				 (VectorType<LHS, T, S> && ScalarType<RHS, T>)     ||
				 (VectorType<LHS, T, S> && std::same_as<RHS, T>) ||
				 (ScalarType<LHS, T>    && VectorType<RHS, T, S>)  ||
				 (std::same_as<LHS, T>  && VectorType<RHS, T, S>)
	{
		if constexpr (std::same_as<LHS, T>)
		{
			return Scalar<T>(lhs) * std::forward<RHS&&>(rhs);
		}

		else if constexpr (std::same_as<RHS, T>)
		{
			return std::forward<LHS&&>(lhs) * Scalar<T>(rhs);
		}

		else
		{
			return { ShaderGraph::PushExpression<OperatorExpression>(Vector<T, S>::ValueType,
														std::forward<LHS&&>(lhs).source(),
														Operator::MULTIPLY,
														std::forward<RHS&&>(rhs).source()) };
		}
	}

	/// Vector division operator
	/**
	* The following divisions are supported:
	* vector / vector
	* vector / scalar
	* scalar / vector
	* vector / T
	*      T / vector
	*/
	template<typename LHS, typename RHS>
	friend Vector<T, S> operator/(LHS&& lhs, RHS&& rhs)
		requires (VectorType<LHS, T, S> && VectorType<RHS, T, S>) ||
				 (VectorType<LHS, T, S> && ScalarType<RHS, T>)    ||
				 (VectorType<LHS, T, S> && std::same_as<RHS, T>)  ||
				 (ScalarType<LHS, T>    && VectorType<RHS, T, S>) ||
				 (std::same_as<LHS, T>  && VectorType<RHS, T, S>)
	{
		if constexpr (std::same_as<LHS, T>)
		{
			return Scalar<T>(lhs) / std::forward<RHS&&>(rhs);
		}
		else if constexpr (std::same_as<RHS, T>)
		{
			return std::forward<LHS&&>(lhs) / Scalar<T>(rhs);
		}
		else
		{
			return { ShaderGraph::PushExpression<OperatorExpression>(Vector<T, S>::ValueType,
														std::forward<LHS&&>(lhs).source(),
														Operator::DIVIDE,
														std::forward<RHS&&>(rhs).source()) };
		}
	}

	/// Vector addition operator
	/**
	* The following additions are supported:
	* vector + vector
	* vector + scalar
	* scalar + vector
	* vector + T
	*      T + vector
	*/
	template<typename LHS, typename RHS>
	friend Vector<T, S> operator+(LHS&& lhs, RHS&& rhs)
		requires (VectorType<LHS, T, S> && VectorType<RHS, T, S>) ||
				 (VectorType<LHS, T, S> && ScalarType<RHS, T>)    ||
				 (VectorType<LHS, T, S> && std::same_as<RHS, T>)  ||
				 (ScalarType<LHS, T>    && VectorType<RHS, T, S>) ||
				 (std::same_as<LHS, T>  && VectorType<RHS, T, S>)
	{
		if constexpr (std::same_as<LHS, T>)
		{
			return Scalar<T>(lhs) + std::forward<RHS&&>(rhs);
		}
		else if constexpr (std::same_as<RHS, T>)
		{
			return std::forward<LHS&&>(lhs) + Scalar<T>(rhs);
		}
		else
		{
			return { ShaderGraph::PushExpression<OperatorExpression>(Vector<T, S>::ValueType,
														std::forward<LHS&&>(lhs).source(),
														Operator::ADD,
														std::forward<RHS&&>(rhs).source()) };
		}
	}

	/// Vector subtraction operator
	/**
	* The following additions are supported:
	* vector - vector
	* vector - scalar
	* scalar - vector
	* vector - T
	*      T - vector
	*/
	template<typename LHS, typename RHS>
	friend Vector<T, S> operator-(LHS&& lhs, RHS&& rhs)
		requires (VectorType<LHS, T, S> && VectorType<RHS, T, S>) ||
				 (VectorType<LHS, T, S> && ScalarType<RHS, T>)    ||
				 (VectorType<LHS, T, S> && std::same_as<RHS, T>)  ||
				 (ScalarType<LHS, T>    && VectorType<RHS, T, S>) ||
				 (std::same_as<LHS, T>  && VectorType<RHS, T, S>)
	{
		if constexpr (std::same_as<LHS, T>)
		{
			return Scalar<T>(lhs) - std::forward<RHS&&>(rhs);
		}
		else if constexpr (std::same_as<RHS, T>)
		{
			return std::forward<LHS&&>(lhs) - Scalar<T>(rhs);
		}
		else
		{
			return { ShaderGraph::PushExpression<OperatorExpression>(Vector<T, S>::ValueType,
														std::forward<LHS&&>(lhs).source(),
														Operator::SUBTRACT,
														std::forward<RHS&&>(rhs).source()) };
		}
	}
};


using float2 = Vector<float, 2>;
using float3 = Vector<float, 3>;
using float4 = Vector<float, 4>;

using int2 = Vector<int, 2>;
using int3 = Vector<int, 3>;
using int4 = Vector<int, 4>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_VECTOR_HPP
