#ifndef CORAL_SHADERLANGUAGE_SCALAR_HPP
#define CORAL_SHADERLANGUAGE_SCALAR_HPP

#include <Coral/ShaderLanguage/Value.hpp>

#include <Coral/ShaderLanguage/ShaderGraph.hpp>

#include <concepts>

namespace Coral::ShaderLanguage
{
/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar;

template<typename S, typename T>
concept ScalarType = std::same_as<std::remove_cvref_t<S>, Scalar<T>>;

template<typename T>
struct TypeTraits;


template<>
struct TypeTraits<Scalar<float>>
{
	constexpr static ValueType ValueType = ValueType::FLOAT;
};


template<>
struct TypeTraits<Scalar<int>>
{
	constexpr static ValueType ValueType = ValueType::INT;
};


template<>
struct TypeTraits<Scalar<bool>>
{
	constexpr static ValueType ValueType = ValueType::BOOL;
};

/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar : Value
{
	using Value::Value;

	static constexpr ValueType ValueType = TypeTraits<Scalar<T>>::ValueType;

	/// Create a new scalar from a constant
	Scalar(T v)
		: Value(ShaderGraph::PushExpression<ConstantExpression<T>>(v))
	{
	}

	Scalar(const Scalar& other)
		: Value(ShaderGraph::PushExpression<ConstructorExpression>(Scalar<T>::ValueType, other.source()))
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
		return { ShaderGraph::PushExpression<CastExpression>(Scalar<U>::ValueType, source()) };
	}

	// ----------------------------------------------------------------------------------------------------------------
    //                                           Scalar assignment operators                                          
    // ----------------------------------------------------------------------------------------------------------------

	/// Assign other to this
	Scalar<T>& operator=(const Scalar<T>& other)
	{
		setSource(ShaderGraph::PushExpression<OperatorExpression>(Scalar<T>::ValueType, source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	/// Assign other to this
	Scalar<T>& operator=(Scalar<T>&& other)
	{
		setSource(ShaderGraph::PushExpression<OperatorExpression>(Scalar<T>::ValueType, source(), Operator::ASSIGNMENT, other.source()));
		return *this;
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator*(LHS&& lhs, RHS&& rhs)
		requires (ScalarType<LHS>      && ScalarType<RHS>) ||
	             (ScalarType<LHS>      && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && ScalarType<RHS>)
	{
		return { ShaderGraph::PushExpression<OperatorExpression>(Scalar<T>::ValueType,
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::MULTIPLY,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator/(LHS&& lhs, RHS&& rhs)
		requires (ScalarType<LHS>        && ScalarType<RHS>) ||
	             (ScalarType<LHS>        && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && ScalarType<RHS>)
	{
		return { ShaderGraph::PushExpression<OperatorExpression>(Scalar<T>::ValueType,
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::DIVIDE,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator+(LHS&& lhs, RHS&& rhs)
		requires (ScalarType<LHS>      && ScalarType<RHS>) ||
	             (ScalarType<LHS>      && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && ScalarType<RHS>)
	{
		return { ShaderGraph::PushExpression<OperatorExpression>(Scalar<T>::ValueType,
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::ADD,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

	template<typename LHS, typename RHS>
	friend Scalar<T> operator-(LHS&& lhs, RHS&& rhs)
		requires (ScalarType<LHS>      && ScalarType<RHS>) ||
	             (ScalarType<LHS>      && std::same_as<RHS, T>) ||
		         (std::same_as<LHS, T> && ScalarType<RHS>)
	{

		return { ShaderGraph::PushExpression<OperatorExpression>(Scalar<T>::ValueType,
													Scalar<T>(std::forward<LHS&&>(lhs)).source(),
													Operator::SUBTRACT,
													Scalar<T>(std::forward<RHS&&>(rhs)).source()) };
	}

};


using Float = Scalar<float>;
using Int   = Scalar<int>;
using Bool  = Scalar<bool>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SCALAR_HPP
