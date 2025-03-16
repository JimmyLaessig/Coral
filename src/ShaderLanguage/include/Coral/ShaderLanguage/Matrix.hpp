#ifndef CORAL_SHADERLANGUAGE_MATRIX_HPP
#define CORAL_SHADERLANGUAGE_MATRIX_HPP


#include <Coral/ShaderLanguage/Vector.hpp>


namespace Coral::ShaderLanguage
{
/// Class representing a vector in the shader graph
template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix : public ValueTypeBase
{
public:
	using ValueTypeBase::ValueTypeBase;

	static constexpr ShaderGraph::ValueType toShaderTypeId();

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
		: ValueTypeBase(ShaderGraph::Node::createConstructor(Matrix<T, C, R>::toShaderTypeId(), (Scalar<T>(values).source(), ...)))
	{
	}
};


template<> constexpr ValueType Matrix<float, 3, 3>::toShaderTypeId() { return ValueType::FLOAT3X3; }
template<> constexpr ValueType Matrix<float, 4, 4>::toShaderTypeId() { return ValueType::FLOAT4X4; }


template<typename T, size_t S>
inline Vector<T, S> operator*(const Matrix<T, S, S>& lhs, const Vector<T, S>& rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator*(const Vector<T, S>& lhs, const Matrix<T, S, S>& rhs)
{
	return { ShaderGraph::Node::createOperator(Vector<T, S>::toShaderTypeId(), ShaderGraph::Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


using Float4x4 = Matrix<float, 4, 4>;
using Float3x3 = Matrix<float, 3, 3>;

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_MATRIX_HPP