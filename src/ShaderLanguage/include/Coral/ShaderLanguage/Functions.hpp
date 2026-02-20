#ifndef CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
#define CORAL_SHADERLANGUAGE_FUNCTIONS_HPP


#include <Coral/ShaderLanguage/Matrix.hpp>

#include <concepts>

namespace Coral::ShaderLanguage
{


template<typename T, size_t S>
inline Vector<T, S> normalize(const Vector<T, S>& v)
{
	return { pushExpression<NativeFunctionExpression>(Vector<T, S>::toValueType(),
		                                              "normalize", 
		                                              v.source()) };
}


template<typename T, size_t S>
inline Vector<T, S> normalize(Vector<T, S>&& v)
{
	return { pushExpression<NativeFunctionExpression>(Vector<T, S>::toValueType(),
													  "normalize",
													  std::forward<Vector<T, S>&&>(v).source()) };
}


template<typename Vec1, typename Vec2>
inline Scalar<float> dot(Vec1&& v1, Vec2&& v2)
	requires IsVector<Vec1, float, 2> && IsVector<Vec2, float, 2> ||
             IsVector<Vec1, float, 3> && IsVector<Vec2, float, 3> ||
	         IsVector<Vec1, float, 4> && IsVector<Vec2, float, 4>
{
	return { pushExpression<NativeFunctionExpression>(ValueType::FLOAT,
													  "dot",
													  std::forward<Vec1&&>(v1).source(),
													  std::forward<Vec2&&>(v2).source()) };
}


template<typename Vec1, typename Vec2>
inline Vector<float, 3> cross(Vec1&& v1, Vec2&& v2)
	requires IsVector<Vec1, float, 3> && IsVector<Vec2, float, 3>

{
	return { pushExpression<NativeFunctionExpression>(Vector<float, 3>::toValueType(),
		                                              "cross", 
		                                              std::forward<Vec1&&>(v1).source(), 
		                                              std::forward<Vec2&&>(v2).source()) };
}


template<typename Vec>
inline Scalar<float> length(Vec&& v)
	requires IsVector<Vec, float, 2> ||
	         IsVector<Vec, float, 3> ||
	         IsVector<Vec, float, 4>
{
	return { pushExpression<NativeFunctionExpression>(Scalar<float>::toValueType(),
		                                              "length", 
		                                              std::forward<Vec&&>(v).source()) };
}


template<typename Vec1, typename Vec2>
inline Scalar<float> distance(Vec1&& p1, Vec2&& p2)
	requires IsVector<Vec1, float, 2> && IsVector<Vec2, float, 2> ||
             IsVector<Vec1, float, 3> && IsVector<Vec2, float, 3> ||
             IsVector<Vec1, float, 4> && IsVector<Vec2, float, 4>
{
	return { pushExpression<NativeFunctionExpression>(Scalar<float>::toValueType(),
		                                              "distance", 
													  std::forward<Vec1&&>(p1).source(),
													  std::forward<Vec2&&>(p2).source()) };
}

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
