#ifndef CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
#define CORAL_SHADERLANGUAGE_FUNCTIONS_HPP


#include <Coral/ShaderLanguage/Matrix.hpp>


namespace Coral::ShaderLanguage
{

inline float3 normalize(const float3& v)
{
	return {  std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT3, "normalize", v.source()) };
}


inline Float dot(const float3& v1, const float3& v2)
{
	return {  std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT, "dot", v1.source(), v2.source()) };
}


inline float3 cross(const float3& v1, const float3& v2)
{
	return {  std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT3, "cross", v1.source(), v2.source()) };
}


template<size_t S>
inline Float length(const Vector<float, S>& v)
{
	return { std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT, "length", v.source()) };
}


template<size_t S>
inline Float distance(const Vector<float, S>& p0, const Vector<float, S>& p1)
{
	return {  std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT, "distance", p0.source(), p1.source()) };
}

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
