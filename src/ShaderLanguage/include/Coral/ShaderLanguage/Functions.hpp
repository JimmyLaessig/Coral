#ifndef CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
#define CORAL_SHADERLANGUAGE_FUNCTIONS_HPP


#include <Coral/ShaderLanguage/Matrix.hpp>


namespace Coral::ShaderLanguage
{

inline Float3 normalize(const Float3& v)
{
	return { ShaderGraph::Node::createNativeFunction(ValueType::FLOAT3, "normalize", v.source()) };
}


inline Float dot(const Float3& v1, const Float3& v2)
{
	return { ShaderGraph::Node::createNativeFunction(ValueType::FLOAT, "dot", v1.source(), v2.source()) };
}


inline Float3 cross(const Float3& v1, const Float3& v2)
{
	return { ShaderGraph::Node::createNativeFunction(ValueType::FLOAT3, "cross", v1.source(), v2.source()) };
}


template<size_t S>
inline Float length(const Vector<float, S>& v)
{
	return { ShaderGraph::Node::createNativeFunctionCall(ValueType::FLOAT, "length", v.source()) };
}


template<size_t S>
inline Float distance(const Vector<float, S>& p0, const Vector<float, S>& p1)
{
	return { ShaderGraph::Node::createNativeFunctionCall(ValueType::FLOAT, "distance", p0.source(), p1.source()) };
}

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_FUNCTIONS_HPP
