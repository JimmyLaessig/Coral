#ifndef CORAL_SHADERLANGUAGE_HPP
#define CORAL_SHADERLANGUAGE_HPP


#include <Coral/ShaderGraph/ShaderGraph.hpp>

#include <concepts>


namespace Coral::ShaderLanguage
{

/// Base struct for all return values of shader graph expressions
/**
 * Objects that derive from this type act as stack-allocated wrappers arround a shader graph operation and define the
 * output type of said operation. Objects of this type are widely used as inputs to functions that build up new shader
 * graph operations. For example, the ´Float´ struct is a proxy for a floating-point value that is created by a shader
 * graph operation. The object itself does not hold the actual value but stores a pointer to the shader graph node from
 * which it was calculated. The user is only required to interact with the stack-allocated ShaderGraphResult objects
 * and function calls to build the shader graph. The creation and storage of the actual ShaderGraph is abstracted away
 * by this level of indirection.
 */
struct ValueTypeBase
{
public:

	ValueTypeBase(ShaderGraph::Expression source)
		: mSource(source)
	{}

	// Get the expression from which this value originates
	ShaderGraph::Expression source() const
	{
		return mSource;
	}

	/// Get the type id of the value
	ShaderGraph::ValueTypeId typeId() const
	{ 
		return std::visit([](auto expr) { return expr->outputShaderTypeId(); }, mSource);
	}

private:

	ShaderGraph::Expression mSource;
};


/// Class representing scalar value in the shader graph
template<typename T>
struct Scalar : ValueTypeBase
{
	using ValueTypeBase::ValueTypeBase;

	static constexpr ShaderGraph::ValueTypeId toShaderTypeId();

	/// Create a new scalar from a constant
	Scalar(T v)
		: ValueTypeBase(ShaderGraph::Constant<T>::create(v))
	{
	}

	template<typename U> requires (!std::is_same_v<T, U>)
	explicit operator Scalar<U>() const
	{
		return { ShaderGraph::CastExpression::create(Scalar<U>::toShaderTypeId(), source()) };
	}
};


template<> constexpr ShaderGraph::ValueTypeId Scalar<float>::toShaderTypeId() { return ShaderGraph::ValueTypeId::FLOAT; }
template<> constexpr ShaderGraph::ValueTypeId Scalar<int>::toShaderTypeId() { return ShaderGraph::ValueTypeId::INT; }
template<> constexpr ShaderGraph::ValueTypeId Scalar<bool>::toShaderTypeId() { return ShaderGraph::ValueTypeId::BOOL; }


template<typename T1, typename T2>
ShaderGraph::Expression makeScalarExpression(T2 arg)
{
	if constexpr (std::is_same_v<T2, T1>)
	{
		return Scalar<T1>(arg).source();
	}
	else if constexpr(std::is_same_v<T2, Scalar<T1>>)
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

	static constexpr ShaderGraph::ValueTypeId toShaderTypeId();

	/// Create a new Vector from scalars
	template<typename ...Ts> requires (sizeof...(Ts) == S)
	Vector(Ts... args)
		: ValueTypeBase(ShaderGraph::ConstructorExpression::create(Vector<T, S>::toShaderTypeId(), makeScalarExpression<T>(std::forward<Ts>(args))...))
	{
	}

	/// Create a new Vector from a smaller vector with the scalar values appended at the end
	template<size_t S2, typename ...Ts> requires (sizeof...(Ts) + S2 == S)
	Vector(const Vector<T, S2>& v, Ts... args)
		: ValueTypeBase(ShaderGraph::ConstructorExpression::create(Vector<T, S>::toShaderTypeId(), v.source(), makeScalarExpression<T>(std::forward<Ts>(args))...))
	{
	}

	/// Get the x component of the vector
	Scalar<T> x() const
	{ 
		using namespace ShaderGraph;
		return Scalar<T>(SwizzleExpression::create(Scalar<T>::toShaderTypeId(), Swizzle::X, source()));
	}

	/// Get the y component of the vector
	Scalar<T> y() const
	{ 
		using namespace ShaderGraph; 
		return Scalar<T>(SwizzleExpression::create(Scalar<T>::toShaderTypeId(), Swizzle::Y, source()));
	}

	/// Get the z component of the vector
	Scalar<T> z() const requires(S >= 3)
	{
		using namespace ShaderGraph;
		return Scalar<T>(SwizzleExpression::create(Scalar<T>::toShaderTypeId(), Swizzle::Z, source()));
	}

	/// Get the w component of the vector
	Scalar<T> w() const requires(S >= 4)
	{
		using namespace ShaderGraph; 
		return Scalar<T>(SwizzleExpression::create(Scalar<T>::toShaderTypeId(), Swizzle::W, source()));
	}

	/// Get the xy components of the vector
	Vector<T, 2> xy() const					  
	{ 
		using namespace ShaderGraph;
		return Vector<T, 2>{ SwizzleExpression::create(Vector<T, 2>::toShaderTypeId(), Swizzle::XY, source()) };
	}

	/// Get the xyw components of the vector
	Vector<T, 3> xyz() const requires(S >= 3)
	{
		using namespace ShaderGraph;
		return Vector<T, 3>{ SwizzleExpression::create(Vector<T, 3>::toShaderTypeId(), Swizzle::XYZ, source()) };
	}
};


template<> constexpr ShaderGraph::ValueTypeId Vector<float, 2>::toShaderTypeId() { return ShaderGraph::ValueTypeId::FLOAT2; }
template<> constexpr ShaderGraph::ValueTypeId Vector<float, 3>::toShaderTypeId() { return ShaderGraph::ValueTypeId::FLOAT3; }
template<> constexpr ShaderGraph::ValueTypeId Vector<float, 4>::toShaderTypeId() { return ShaderGraph::ValueTypeId::FLOAT4; }

template<> constexpr ShaderGraph::ValueTypeId Vector<int, 2>::toShaderTypeId() { return ShaderGraph::ValueTypeId::INT2; }
template<> constexpr ShaderGraph::ValueTypeId Vector<int, 3>::toShaderTypeId() { return ShaderGraph::ValueTypeId::INT3; }
template<> constexpr ShaderGraph::ValueTypeId Vector<int, 4>::toShaderTypeId() { return ShaderGraph::ValueTypeId::INT4; }


/// Class representing a vector in the shader graph
template<typename T, size_t C, size_t R> requires (C == R) && (C == 3 || C == 4)
struct Matrix : public ValueTypeBase
{
public:
	using ValueTypeBase::ValueTypeBase;

	static constexpr ShaderGraph::ValueTypeId toShaderTypeId();

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
		: ValueTypeBase(ShaderGraph::ConstructorExpression::create(Matrix<T, C, R>::toShaderTypeId(), (Scalar<T>(values).source(), ...)))
	{
	}
};


template<> constexpr ShaderGraph::ValueTypeId Matrix<float, 3, 3>::toShaderTypeId() { return ShaderGraph::ValueTypeId::FLOAT3X3; }
template<> constexpr ShaderGraph::ValueTypeId Matrix<float, 4, 4>::toShaderTypeId() { return ShaderGraph::ValueTypeId::FLOAT4X4; }


//---------------------------------------------------------------------------------------------------------------------
// Scalar x Scalar operators
//---------------------------------------------------------------------------------------------------------------------

template<typename T>
inline Scalar<T> operator*(Scalar<T> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Scalar<T>::toShaderTypeId(), Operator::MULTIPLY, lhs.source(), rhs.source())};
}


template<typename T>
inline Scalar<T> operator/(Scalar<T> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Scalar<T>::toShaderTypeId(), Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<T> operator+(Scalar<T> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Scalar<T>::toShaderTypeId(), Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<T> operator-(Scalar<T> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Scalar<T>::toShaderTypeId(), Operator::SUBTRACT, lhs.source(), rhs.source()) };
}


template<typename T>
inline Scalar<T> operator*(Scalar<T> lhs, T rhs) { return lhs * Scalar(rhs); }


template<typename T>
inline Scalar<T> operator/(Scalar<T> lhs, T rhs){ return lhs / Scalar(rhs); }


template<typename T>
inline Scalar<T> operator+(Scalar<T> lhs,T rhs) { return lhs + Scalar(rhs); }


template<typename T>
inline Scalar<T> operator-(Scalar<T> lhs, T rhs) { return lhs - Scalar(rhs); }


template<typename T>
inline Scalar<T> operator*(T lhs, Scalar<T> rhs) { return Scalar(lhs) * rhs; }
						   								
						   								
template<typename T>	   								
inline Scalar<T> operator/(T lhs, Scalar<T> rhs) { return Scalar(lhs) / rhs; }
						   								 
						   								 
template<typename T>	   								 
inline Scalar<T> operator+(T lhs, Scalar<T> rhs) { return Scalar(lhs) + rhs; }
						   								 
						   								 
template<typename T>	   								 
inline Scalar<T> operator-(T lhs, Scalar<T> rhs) { return Scalar(lhs) - rhs; }


//---------------------------------------------------------------------------------------------------------------------
// Vector x Scalar operators
//---------------------------------------------------------------------------------------------------------------------

template<typename T, size_t S>
inline Vector<T, S> operator*(Vector<T, S> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, Scalar<T> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::SUBTRACT, lhs.source(), rhs.source()) };
}

template<typename T, size_t S>
inline Vector<T, S> operator*(Vector<T, S> lhs, T rhs) { return lhs * Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, T rhs) { return lhs / Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, T rhs) { return lhs + Scalar(rhs); }


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, T rhs) { return lhs - Scalar(rhs); }


//---------------------------------------------------------------------------------------------------------------------
// Scalar x Vector operators
//---------------------------------------------------------------------------------------------------------------------

template<typename T, size_t S>
inline Vector<T, S> operator*(Scalar<T> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>																	  
inline Vector<T, S> operator/(Scalar<T> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Scalar<T> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::ADD, lhs.source(), rhs.source()) };
}
		

template<typename T, size_t S>
inline Vector<T, S> operator-(Scalar<T> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::SUBTRACT, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator*(T lhs, Vector<T, S> rhs) { return Scalar(lhs) * rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator/(T lhs, Vector<T, S> rhs) { return Scalar(lhs) / rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator+(T lhs, Vector<T, S> rhs) { return Scalar(lhs) + rhs; }


template<typename T, size_t S>
inline Vector<T, S> operator-(T lhs, Vector<T, S> rhs) { return Scalar(lhs) - rhs; }

//---------------------------------------------------------------------------------------------------------------------
// Vector x Vector operators
//---------------------------------------------------------------------------------------------------------------------

template<typename T, size_t S>
inline Vector<T, S> operator*(Vector<T, S> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator/(Vector<T, S> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::DIVIDE, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator+(Vector<T, S> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::ADD, lhs.source(), rhs.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> operator-(Vector<T, S> lhs, Vector<T, S> rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::SUBTRACT, lhs.source(), rhs.source()) };
}

//---------------------------------------------------------------------------------------------------------------------
// Matrix * Vector operator
//---------------------------------------------------------------------------------------------------------------------

template<typename T, size_t S>
inline Vector<T, S> operator*(const Matrix<T, S, S>& lhs, const Vector<T, S>& rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::MULTIPLY, lhs.source(), rhs.source()) };
}

//---------------------------------------------------------------------------------------------------------------------
// Vector * Matrix operator
//---------------------------------------------------------------------------------------------------------------------

template<typename T, size_t S>
inline Vector<T, S> operator*(const Vector<T, S>& lhs, const Matrix<T, S, S>& rhs)
{
	using namespace ShaderGraph;
	return { OperatorExpression::create(Vector<T, S>::toShaderTypeId(), Operator::MULTIPLY, lhs.source(), rhs.source()) };
}


using Float = Scalar<float>;
using Float2 = Vector<float, 2>;
using Float3 = Vector<float, 3>;
using Float4 = Vector<float, 4>;

using Int = Scalar<int>;
using Int2 = Vector<int, 2>;
using Int3 = Vector<int, 3>;
using Int4 = Vector<int, 4>;

using Float4x4 = Matrix<float, 4, 4>;
using Float3x3 = Matrix<float, 3, 3>;


inline Float3 normalize(const Float3& v)
{
	using namespace ShaderGraph;
	return { NativeFunctionExpression::create(ValueTypeId::FLOAT3, "normalize", v.source()) };
}


inline Float dot(const Float3& v1, const Float3& v2)
{
	using namespace ShaderGraph;
	return { NativeFunctionExpression::create(ValueTypeId::FLOAT, "dot", v1.source(), v2.source()) };
}


inline Float3 cross(const Float3& v1, const Float3& v2)
{
	using namespace ShaderGraph;
	return { NativeFunctionExpression::create(ValueTypeId::FLOAT3, "cross", v1.source(), v2.source()) };
}


template<size_t S>
inline Float length(const Vector<float, S>& v)
{
	using namespace ShaderGraph;
	return { NativeFunctionExpression::create(ValueTypeId::FLOAT, "length", v.source()) };
}


template<size_t S>
inline Float distance(const Vector<float, S>& p0, const Vector<float, S>& p1)
{
	using namespace ShaderGraph;
	return { NativeFunctionExpression::create(ValueTypeId::FLOAT, "distance", p0.source(), p1.source())};
}


template<typename T>
inline T Attribute(std::string_view name)
{
	using namespace ShaderGraph;
	return { AttributeExpression::create(T::toShaderTypeId(), name)};
}


template<typename T>
inline T Parameter(std::string_view name)
{
	using namespace ShaderGraph;
	return { ShaderGraph::ParameterExpression::create(T::toShaderTypeId(), name) };
}

//---------------------------------------------------------------------------------------------------------------------
// Texture
//---------------------------------------------------------------------------------------------------------------------

struct Sampler2D : ValueTypeBase
{
	using ValueTypeBase::ValueTypeBase;

	static constexpr ShaderGraph::ValueTypeId toShaderTypeId() { return ShaderGraph::ValueTypeId::SAMPLER2D; }

	Float4 sample(Float2 uv)
	{
		using namespace ShaderGraph;
		return { ShaderGraph::NativeFunctionExpression::create(ValueTypeId::FLOAT4, "texture", source(), uv.source()) };
	}

	Int2 size(Scalar<int> lod)
	{
		using namespace ShaderGraph;
		return { ShaderGraph::NativeFunctionExpression::create(ValueTypeId::FLOAT3, "textureSize", source(), lod.source()) };
	}
};

namespace DefaultSemantics = Coral::ShaderLanguage::ShaderGraph::DefaultSemantics;

} // namespace ShaderLanguage 

namespace csl = Coral::ShaderLanguage;

#endif // !CORAL_SHADERLANGUAGE_HPP