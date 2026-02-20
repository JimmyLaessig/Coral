#ifndef CORAL_SHADERLANGUAGE_MATRIX_HPP
#define CORAL_SHADERLANGUAGE_MATRIX_HPP


#include <Coral/ShaderLanguage/Vector.hpp>

#include <Coral/ShaderLanguage/Expression.hpp>

namespace Coral::ShaderLanguage
{

template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix;

template<typename M, typename T, size_t C, size_t R>
concept IsMatrix = std::derived_from<std::remove_cvref_t<M>, Matrix<T, C, R>>;

/// Class representing a matrix value
template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix : public Value
{
public:

	using Value::Value;

	static constexpr ValueType toValueType();

	/// Create a new Matrix with the diagonal filled with the value
	Matrix(T v)
	{
		if constexpr (C == 3)
		{
			Matrix::Matrix(v, 0, 0,
						   0, v, 0,
						   0, 0, v);
		}
		else
		{
			Matrix::Matrix(v, 0, 0, 0,
				           0, v, 0, 0,
				           0, 0, v, 0,
				           0, 0, 0, v);
		}
	}

	template<typename ...Ts> requires (sizeof...(Ts) == C * R)
	Matrix(Ts... values)
		: Value(pushExpression<ConstructorExpression>(Matrix<T, C, R>::toValueType(), (Scalar<T>(values).source(), ...)))
	{
	}

	Matrix(const Matrix& other)
		: Value(pushExpression<ConstructorExpression>(Matrix::toValueType(), other.source()))
	{
	}

	Matrix(Matrix&& other)
		: Value(pushExpression<ConstructorExpression>(Matrix::toValueType(), other.source()))
	{
	}

	Matrix& operator=(const Matrix& other)
	{
		mSource = pushExpression<OperatorExpression>(Matrix::toValueType(), source(), Operator::ASSIGNMENT, other.source());
		return *this;
	}

	Matrix& operator=(Matrix&& other)
	{
		mSource = pushExpression<OperatorExpression>(Matrix::toValueType(), source(), Operator::ASSIGNMENT, other.source());
		return *this;
	}

	/// Matrix - vector multiplication operator
	template<typename LHS, typename RHS>
	friend Vector<T, C> operator*(LHS&& lhs, RHS&& rhs)
		requires (IsMatrix<LHS, T, C, R> && IsVector<RHS, T, C>) ||
				 (IsVector<LHS, T, C>    && IsMatrix<RHS, T, C, R>)
	{
		return { pushExpression<OperatorExpression>(Vector<T, C>::toValueType(),
													std::forward<LHS&&>(lhs).source(),
													Operator::MULTIPLY,
													std::forward<RHS&&>(rhs).source()) };
	}

	/// Matrix * Matrix operator
	template<typename LHS, typename RHS>
	friend Matrix<T, C, R> operator*(LHS&& lhs, RHS&& rhs)
		requires (IsMatrix<LHS, T, C, R> && IsMatrix<RHS, T, C, R>)
	{
		return { pushExpression<OperatorExpression>(Vector<T, C>::toValueType(),
													std::forward<LHS&&>(lhs).source(),
													Operator::MULTIPLY,
													std::forward<RHS&&>(rhs).source()) };
	}
};

template<> constexpr ValueType Matrix<float, 3, 3>::toValueType() { return ValueType::FLOAT3X3; }
template<> constexpr ValueType Matrix<float, 4, 4>::toValueType() { return ValueType::FLOAT4X4; }

using Float4x4 = Matrix<float, 4, 4>;
using Float3x3 = Matrix<float, 3, 3>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_MATRIX_HPP