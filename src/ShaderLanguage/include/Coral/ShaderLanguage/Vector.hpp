#ifndef CORAL_SHADERLANGUAGE_VECTOR_HPP
#define CORAL_SHADERLANGUAGE_VECTOR_HPP

#include <Coral/ShaderLanguage/Scalar.hpp>

namespace Coral::ShaderLanguage
{

template<typename T, size_t S>  requires (S > 1)
struct Vector;

template<typename V, typename T, size_t S>
concept IsVector = std::derived_from<std::remove_cvref_t<V>, Vector<T, S>>;

/// Class representing a vector in the shader graph
template<typename T, size_t S> requires (S > 1)
struct Vector : public Value
{

public:

	typedef T value_type;
	constexpr static size_t length = S;

	using Value::Value;

	static constexpr ValueType toValueType();

	/// Create a new Vector from scalars
	template<typename ...Ts> requires (sizeof...(Ts) == S)
	Vector(Ts... args)
		: Value(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toValueType(), (Scalar<T>(args).source(), ...)))
	{
	}

	/// Create a new Vector from a smaller vector with the scalar values appended at the end
	template<size_t S2, typename ...Ts> requires (sizeof...(Ts) + S2 == S)
	Vector(const Vector<T, S2>& v, Ts... args)
		: Value(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toValueType(), v.source(), (Scalar<T>(args).source(), ...)))
	{
	}

	Vector(const Vector& other)
		: Value(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toValueType(), other.source()))
	{
	}

	Vector(Vector&& other)
		: Value(ShaderModule::current()->addExpression<ConstructorExpression>(Vector<T, S>::toValueType(), other.source()))
	{
	}

	Vector<T, S>& operator=(const Vector<T, S>& other)
	{
		setSource(ShaderModule::current()->addExpression<OperatorExpression>(Vector<T, S>::toValueType(), source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	Vector<T, S>& operator=(Vector<T, S>&& other)
	{
		setSource(ShaderModule::current()->addExpression<OperatorExpression>(Vector<T, S>::toValueType(), source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	/// Get the x component of the vector
	Scalar<T> x() const
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toValueType(), Swizzle::X, source()) };
	}

	/// Get the y component of the vector
	Scalar<T> y() const
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toValueType(), Swizzle::Y, source()) };
	}

	/// Get the z component of the vector
	Scalar<T> z() const requires(S >= 3)
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toValueType(), Swizzle::Z, source()) };
	}

	/// Get the w component of the vector
	Scalar<T> w() const requires(S >= 4)
	{
		return { std::make_shared<SwizzleExpression>(Scalar<T>::toValueType(), Swizzle::W, source()) };
	}

	/// Get the xy components of the vector
	Vector<T, 2> xy() const
	{
		return { std::make_shared<SwizzleExpression>(Vector<T, 2>::toValueType(), Swizzle::XY, source()) };
	}

	/// Get the xyz components of the vector
	Vector<T, 3> xyz() const requires(S >= 3)
	{
		return { std::make_shared<SwizzleExpression>(Vector<T, 3>::toValueType(), Swizzle::XYZ, source()) };
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
		requires (IsVector<LHS, T, S>  && IsVector<RHS, T, S>)  ||
	             (IsVector<LHS, T, S>  && IsScalar<RHS, T>)     ||
				 (IsVector<LHS, T, S>  && std::same_as<RHS, T>) || 
				 (IsScalar<LHS, T>     && IsVector<RHS, T, S>)  || 
				 (std::same_as<LHS, T> && IsVector<RHS, T, S>)
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
			return { pushExpression<OperatorExpression>(Vector<T, S>::toValueType(),
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
		requires (IsVector<LHS, T, S>  && IsVector<RHS, T, S>) ||
				 (IsVector<LHS, T, S>  && IsScalar<RHS, T>) ||
				 (IsVector<LHS, T, S>  && std::same_as<RHS, T>) ||
				 (IsScalar<LHS, T>     && IsVector<RHS, T, S>) ||
				 (std::same_as<LHS, T> && IsVector<RHS, T, S>)
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
			return { pushExpression<OperatorExpression>(Vector<T, S>::toValueType(),
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
		requires (IsVector<LHS, T, S>  && IsVector<RHS, T, S>) ||
	             (IsVector<LHS, T, S>  && IsScalar<RHS, T>) ||
		         (IsVector<LHS, T, S>  && std::same_as<RHS, T>) ||
		         (IsScalar<LHS, T>     && IsVector<RHS, T, S>) ||
		         (std::same_as<LHS, T> && IsVector<RHS, T, S>)
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
			return { pushExpression<OperatorExpression>(Vector<T, S>::toValueType(),
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
		requires (IsVector<LHS, T, S>  && IsVector<RHS, T, S>) ||
	             (IsVector<LHS, T, S>  && IsScalar<RHS, T>) ||
		         (IsVector<LHS, T, S>  && std::same_as<RHS, T>) ||
		         (IsScalar<LHS, T>     && IsVector<RHS, T, S>) ||
		         (std::same_as<LHS, T> && IsVector<RHS, T, S>)
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
			return { pushExpression<OperatorExpression>(Vector<T, S>::toValueType(),
														std::forward<LHS&&>(lhs).source(),
														Operator::SUBTRACT,
														std::forward<RHS&&>(rhs).source()) };
		}
	}
};


template<> constexpr ValueType Vector<float, 2>::toValueType() { return ValueType::FLOAT2; }
template<> constexpr ValueType Vector<float, 3>::toValueType() { return ValueType::FLOAT3; }
template<> constexpr ValueType Vector<float, 4>::toValueType() { return ValueType::FLOAT4; }

template<> constexpr ValueType Vector<int, 2>::toValueType() { return ValueType::INT2; }
template<> constexpr ValueType Vector<int, 3>::toValueType() { return ValueType::INT3; }
template<> constexpr ValueType Vector<int, 4>::toValueType() { return ValueType::INT4; }


using float2 = Vector<float, 2>;
using float3 = Vector<float, 3>;
using float4 = Vector<float, 4>;

using int2 = Vector<int, 2>;
using int3 = Vector<int, 3>;
using int4 = Vector<int, 4>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_VECTOR_HPP
