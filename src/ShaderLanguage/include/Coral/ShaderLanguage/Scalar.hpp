#ifndef CORAL_SHADERLANGUAGE_SCALAR_HPP
#define CORAL_SHADERLANGUAGE_SCALAR_HPP

#include <Coral/ShaderLanguage/Value.hpp>

#include <Coral/ShaderLanguage/ShaderModule.hpp>

#include <concepts>

namespace Coral::ShaderLanguage
{
/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar;


template<typename ScalarType, typename T>
concept IsScalar = std::derived_from<std::remove_cvref_t<ScalarType>, Scalar<T>>;


/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar : Value
{
	using Value::Value;

	static constexpr ValueType toValueType()
	{
		if constexpr (std::same_as <T, float>)
		{
			return ValueType::FLOAT;
		}
		else if constexpr (std::same_as <T, int>)
		{
			return ValueType::INT;
		}
		else if constexpr (std::same_as <T, bool>)
		{
			return ValueType::BOOL;
		}
		else
		{
			static_assert(false);
		}
	}

	/// Create a new scalar from a constant
	Scalar(T v)
		: Value(pushExpression<ConstantExpression<T>>(v))
	{
	}

	Scalar(const Scalar& other)
		: Value(pushExpression<ConstructorExpression>(Scalar<T>::toValueType(), other.source()))
	{

	}

	Scalar(Scalar&& other)
		: Value(other.source())
	{

	}

	/// Cast Scalar<U> to Scalar<T>
	template<typename U> requires (!std::is_same_v<T, U>)
	explicit operator Scalar<U>() const
	{
		return { pushExpression<CastExpression>(Scalar<U>::toValueType(), source()) };
	}

	// ----------------------------------------------------------------------------------------------------------------
    //                                           Scalar assignment operators                                          
    // ----------------------------------------------------------------------------------------------------------------

	/// Assign other to this
	Scalar<T>& operator=(const Scalar<T>& other)
	{
		setSource(pushExpression<OperatorExpression>(Scalar<T>::toValueType(), source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	/// Assign other to this
	Scalar<T>& operator=(Scalar<T>&& other)
	{
		setSource(pushExpression<OperatorExpression>(Scalar<T>::toValueType(), source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator*(LHS&& lhs, RHS&& rhs)
		requires (IsScalar<LHS>        && IsScalar<RHS>) ||
	             (IsScalar<LHS>        && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && IsScalar<RHS>)
	{
		return { pushExpression<OperatorExpression>(Scalar<T>::toValueType(),
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::MULTIPLY,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator/(LHS&& lhs, RHS&& rhs)
		requires (IsScalar<LHS>        && IsScalar<RHS>) ||
	             (IsScalar<LHS>        && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && IsScalar<RHS>)
	{
		return { pushExpression<OperatorExpression>(Scalar<T>::toValueType(),
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::DIVIDE,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator+(LHS&& lhs, RHS&& rhs)
		requires (IsScalar<LHS>        && IsScalar<RHS>) ||
	             (IsScalar<LHS>        && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && IsScalar<RHS>)
	{
		return { pushExpression<OperatorExpression>(Scalar<T>::toValueType(),
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::ADD,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator-(LHS&& lhs, RHS&& rhs)
		requires (IsScalar<LHS>        && IsScalar<RHS>) ||
	             (IsScalar<LHS>        && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && IsScalar<RHS>)
	{

		return { pushExpression<OperatorExpression>(Scalar<T>::toValueType(),
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::SUBTRACT,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

};


//template <typename L, typename R>
//Scalar<bool> operator==(L&& lhs, R&& rhs)
//	requires (IsScalar<L> && IsScalar<R>)
//{
//	using T1 = std::decay_t<L> ::value_type;
//	using T2 = std::decay_t<R> ::value_type;
//
//	static_assert(std::same_as<T1, T2>);
//
//	return { pushExpression<OperatorExpression>(Scalar<bool>::toValueType(),
//												Scalar<T1>(std::forward<L&&>(lhs)).source(),
//												Operator::EQUAL,
//												Scalar<T2>(std::forward<R&&>(rhs)).source()) };
//}



using Float = Scalar<float>;
using Int   = Scalar<int>;
using Bool  = Scalar<bool>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SCALAR_HPP
