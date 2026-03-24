#ifndef CORAL_SHADERLANGUAGE_MATRIX_HPP
#define CORAL_SHADERLANGUAGE_MATRIX_HPP


#include <Coral/ShaderLanguage/Vector.hpp>

namespace Coral::ShaderLanguage
{

template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix;

template<typename M, typename T, size_t C, size_t R>
concept MatrixType = std::same_as<std::remove_cvref_t<M>, Matrix<T, C, R>>;

template<>
struct TypeTraits<Matrix<float, 3, 3>>
{
	constexpr static ValueType ValueType = ValueType::FLOAT3X3;
};

template<>
struct TypeTraits<Matrix<float, 4, 4>>
{
	constexpr static ValueType ValueType = ValueType::FLOAT4X4;
};

/// Class representing a matrix value
template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix : public Value
{
public:

	Matrix() 
		: Matrix(T(1))
	{}

	using Value::Value;

	static constexpr ValueType ValueType = TypeTraits<Matrix<T, C, R>>::ValueType;


	/// Create a new Matrix with the diagonal filled with the value
	Matrix(T v)  requires (C == 3)
		: Matrix(v, 0, 0,
			     0, v, 0,
			     0, 0, v)
	{
	}


	Matrix(T v) requires (C == 4)
		: Matrix(v, 0, 0, 0,
			     0, v, 0, 0,
			     0, 0, v, 0,
			     0, 0, 0, v)
	{
	}


	template<typename ...Ts> requires (sizeof...(Ts) == C * R)
	Matrix(Ts... args)
		: Value(Node::create(ConstructorExpression(Matrix::ValueType), Scalar<T>(args).node()...))
	{
	}

	Matrix(const Matrix& other)
		: Value(Node::create(ConstructorExpression(Matrix::ValueType), other.node()))
	{
	}

	Matrix(Matrix&& other)
		: Value(Node::create(ConstructorExpression(Matrix::ValueType), other.node()))
	{
	}

	Matrix& operator=(const Matrix& other)
	{
		mNode = Node::create(OperatorExpression(Matrix::ValueType, Operator::ASSIGNMENT), node(), other.node());
		return *this;
	}

	Matrix& operator=(Matrix&& other)
	{
		mNode = Node::create(OperatorExpression(Matrix::ValueType, Operator::ASSIGNMENT), node(),other.node());
		return *this;
	}

	/// Matrix - vector multiplication operator
	template<typename LHS, typename RHS>
	friend Vector<T, C> operator*(LHS&& lhs, RHS&& rhs)
		requires (MatrixType<LHS, T, C, R> && VectorType<RHS, T, C>) ||
				 (VectorType<LHS, T, C>    && MatrixType<RHS, T, C, R>)
	{
		return { Node::create(OperatorExpression(Vector<T, C>::ValueType, Operator::MULTIPLY),
							  std::forward<LHS&&>(lhs).node(),
							  std::forward<RHS&&>(rhs).node()) };
	}

	/// Matrix * Matrix operator
	template<typename LHS, typename RHS>
	friend Matrix<T, C, R> operator*(LHS&& lhs, RHS&& rhs)
		requires (MatrixType<LHS, T, C, R> && MatrixType<RHS, T, C, R>)
	{
		return { Node::create(OperatorExpression(Vector<T, C>::ValueType, Operator::MULTIPLY),
							  std::forward<LHS&&>(lhs).node(),
							  std::forward<RHS&&>(rhs).node()) };
	}
};

using Float4x4 = Matrix<float, 4, 4>;
using Float3x3 = Matrix<float, 3, 3>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_MATRIX_HPP