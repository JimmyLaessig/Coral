#ifndef CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
#define CORAL_SHADERLANGUAGE_FUNCTIONS_HPP


#include <Coral/ShaderLanguage/Matrix.hpp>

#include <concepts>

namespace Coral::ShaderLanguage
{


template<typename T, size_t S>
inline Vector<T, S> normalize(const Vector<T, S>& v)
{
    return { Node::create(NativeFunctionExpression(Vector<T, S>::ValueType, NativeFunction::NORMALIZE), 
                          v.node()) };
}


template<typename T, size_t S>
inline Vector<T, S> normalize(Vector<T, S>&& v)
{
    return { Node::create(NativeFunctionExpression(Vector<T, S>::ValueType, NativeFunction::NORMALIZE),
                          std::forward<Vector<T, S>&&>(v).node()) };
}


template<typename Vec1, typename Vec2>
inline Scalar<float> dot(Vec1&& v1, Vec2&& v2)
    requires VectorType<Vec1, float, 2> && VectorType<Vec2, float, 2> ||
             VectorType<Vec1, float, 3> && VectorType<Vec2, float, 3> ||
             VectorType<Vec1, float, 4> && VectorType<Vec2, float, 4>
{
    return { Node::create(NativeFunctionExpression(ValueType::FLOAT, NativeFunction::DOT),
                          std::forward<Vec1&&>(v1).node(),
                          std::forward<Vec2&&>(v2).node()) };
}


template<typename Vec1, typename Vec2>
inline Vector<float, 3> cross(Vec1&& v1, Vec2&& v2)
    requires VectorType<Vec1, float, 3> && VectorType<Vec2, float, 3>

{
    return { Node::create(NativeFunctionExpression(Vector<float, 3>::ValueType, NativeFunction::CROSS),
                          std::forward<Vec1&&>(v1).node(), 
                          std::forward<Vec2&&>(v2).node()) };
}


template<typename Vec>
inline Scalar<float> length(Vec&& v)
    requires VectorType<Vec, float, 2> ||
             VectorType<Vec, float, 3> ||
             VectorType<Vec, float, 4>
{
    return { Node::create(NativeFunctionExpression(Scalar<float>::ValueType, NativeFunction::LENGTH),
                          std::forward<Vec&&>(v).node()) };
}


template<typename Vec1, typename Vec2>
inline Scalar<float> distance(Vec1&& p1, Vec2&& p2)
    requires VectorType<Vec1, float, 2> && VectorType<Vec2, float, 2> ||
             VectorType<Vec1, float, 3> && VectorType<Vec2, float, 3> ||
             VectorType<Vec1, float, 4> && VectorType<Vec2, float, 4>
{
    return { Node::create(NativeFunctionExpression(Scalar<float>::ValueType,NativeFunction::DISTANCE),
                          std::forward<Vec1&&>(p1).node(),
                          std::forward<Vec2&&>(p2).node()) };
}

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
