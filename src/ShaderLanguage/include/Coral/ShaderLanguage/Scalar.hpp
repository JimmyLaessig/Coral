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

template<typename V, typename T>
concept ConstantType = std::same_as<std::remove_cvref_t<V>, T>;

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
        : Value(Node::create(ConstantExpression<T>(v)))
    {
    }

    Scalar(const Scalar& other)
        : Value(Node::create(ConstructorExpression(Scalar<T>::ValueType), other.node()))
    {

    }

    Scalar(Scalar&& other)
        : Value(other.node())
    {

    }

    /// Cast Scalar<U> to Scalar<T>
    template<typename U> requires (!std::is_same_v<T, U>)
    explicit operator Scalar<U>() const
    {
        return { Node::create(CastExpression(Scalar<U>::ValueType), node()) };
    }

    // ----------------------------------------------------------------------------------------------------------------
    //                                           Scalar assignment operators                                          
    // ----------------------------------------------------------------------------------------------------------------

    /// Assign other to this
    Scalar<T>& operator=(const Scalar<T>& other)
    {
        setNode(Node::create(OperatorExpression(Scalar<T>::ValueType, Operator::ASSIGNMENT), 
                               node(), 
                               other.node()));
        return *this;
    }

    /// Assign other to this
    Scalar<T>& operator=(Scalar<T>&& other)
    {
        setNode(Node::create(OperatorExpression(Scalar<T>::ValueType, Operator::ASSIGNMENT), 
                               node(), 
                               other.node()));
        return *this;
    }

    template<typename LHS, typename RHS>
    friend Scalar<T> operator*(LHS&& lhs, RHS&& rhs)
        requires (ScalarType<LHS>      && ScalarType<RHS>) ||
                 (ScalarType<LHS>      && ConstantType<RHS, T>) ||
                 (ConstantType<LHS, T> && ScalarType<RHS>)
    {
        return { Node::create(OperatorExpression(Scalar<T>::ValueType, Operator::MULTIPLY),
                              Scalar<T>(std::forward<LHS&&>(lhs)).node(),
                              Scalar<T>(std::forward<RHS&&>(rhs)).node()) };
    }

    template<typename LHS, typename RHS>
    friend Scalar<T> operator/(LHS&& lhs, RHS&& rhs)
        requires (ScalarType<LHS>        && ScalarType<RHS>) ||
                 (ScalarType<LHS>        && ConstantType<RHS, T>) ||
                 (ConstantType<LHS, T> && ScalarType<RHS>)
    {
        return { Node::create(OperatorExpression(Scalar<T>::ValueType, Operator::DIVIDE),
                              Scalar<T>(std::forward<LHS&&>(lhs)).node(),
                              Scalar<T>(std::forward<RHS&&>(rhs)).node()) };
    }

    template<typename LHS, typename RHS>
    friend Scalar<T> operator+(LHS&& lhs, RHS&& rhs)
        requires (ScalarType<LHS>      && ScalarType<RHS>) ||
                 (ScalarType<LHS>      && ConstantType<RHS, T>) ||
                 (ConstantType<LHS, T> && ScalarType<RHS>)
    {
        return { Node::create(OperatorExpression(Scalar<T>::ValueType, Operator::ADD),
                              Scalar<T>(std::forward<LHS&&>(lhs)).node(),
                              Scalar<T>(std::forward<RHS&&>(rhs)).node()) };
    }

    template<typename LHS, typename RHS>
    friend Scalar<T> operator-(LHS&& lhs, RHS&& rhs)
        requires (ScalarType<LHS>      && ScalarType<RHS>) ||
                 (ScalarType<LHS>      && ConstantType<RHS, T>) ||
                 (ConstantType<LHS, T> && ScalarType<RHS>)
    {
        return { Node::create(OperatorExpression(Scalar<T>::ValueType, Operator::SUBTRACT),
                              Scalar<T>(std::forward<LHS&&>(lhs)).node(),
                              Scalar<T>(std::forward<RHS&&>(rhs)).node()) };
    }

};


using Float = Scalar<float>;
using Int   = Scalar<int>;
using Bool  = Scalar<bool>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SCALAR_HPP
