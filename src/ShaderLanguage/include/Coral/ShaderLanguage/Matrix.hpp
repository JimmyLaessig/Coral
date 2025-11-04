#ifndef CORAL_SHADERLANGUAGE_MATRIX_HPP
#define CORAL_SHADERLANGUAGE_MATRIX_HPP


#include <Coral/ShaderLanguage/Vector.hpp>


namespace Coral::ShaderLanguage
{
/// Class representing a vector in the shader graph
template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix : public Variable
{
public:

	using Variable::Variable;

	static constexpr ValueType toShaderTypeId();

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
		: Variable(ShaderModule::current()->addExpression<ConstructorExpression>(Matrix<T, C, R>::toShaderTypeId(), (Scalar<T>(values).source(), ...)))
	{
	}

	Matrix& operator=(const Matrix& other)
	{
		mSource = ShaderModule::current()->addExpression<OperatorExpression>(Matrix::toShaderTypeId(), other.source());
		return *this;
	}

	Matrix& operator=(Matrix&& other)
	{
		mSource = ShaderModule::current()->addExpression<OperatorExpression>(Matrix::toShaderTypeId(), other.source());
		return *this;
	}
};

template<> constexpr ValueType Matrix<float, 3, 3>::toShaderTypeId() { return ValueType::FLOAT3X3; }
template<> constexpr ValueType Matrix<float, 4, 4>::toShaderTypeId() { return ValueType::FLOAT4X4; }


template<typename MatrixType, typename VectorType>
VectorType operator*(MatrixType&& lhs, VectorType&& rhs)
requires(
(std::derived_from<std::decay_t<MatrixType>, Matrix<float, 4, 4>> && std::derived_from<std::decay_t<VectorType>, Vector<float, 4>>) ||
(std::derived_from<std::decay_t<MatrixType>, Matrix<float, 3, 3>> && std::derived_from<std::decay_t<VectorType>, Vector<float, 3>>)
)
{
	return std::decay_t<VectorType>(ShaderModule::current()->addExpression<OperatorExpression>(std::decay_t<VectorType>::toShaderTypeId(), lhs.source(), Operator::MULTIPLY, rhs.source()));
}





template<typename T, size_t S>
inline Vector<T, S> operator*(const Vector<T, S>& lhs, const Matrix<T, S, S>& rhs)
{
	return { ShaderModule::current()->addExpression<OperatorExpression>(Vector<T, S>::toShaderTypeId(), lhs.source(), Operator::MULTIPLY, rhs.source()) };
}


using Float4x4 = Matrix<float, 4, 4>;
using Float3x3 = Matrix<float, 3, 3>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_MATRIX_HPP