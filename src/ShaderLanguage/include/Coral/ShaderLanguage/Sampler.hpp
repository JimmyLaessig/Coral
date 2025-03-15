#ifndef CORAL_SHADERLANGUAGE_SAMPLER_HPP
#define CORAL_SHADERLANGUAGE_SAMPLER_HPP

#include <Coral/ShaderLanguage/Vector.hpp>

namespace Coral::ShaderLanguage
{
struct Sampler2D : ValueTypeBase
{
	using ValueTypeBase::ValueTypeBase;

	static constexpr ValueType toShaderTypeId() { return ValueType::SAMPLER2D; }

	Float4 sample(Float2 uv)
	{
		return { ShaderGraph::Node::createNativeFunction(ValueType::FLOAT4, "texture", source(), uv.source()) };
	}

	Int2 size(Int lod)
	{
		return { ShaderGraph::Node::createNativeFunction(ValueType::FLOAT3, "textureSize", source(), lod.source()) };
	}
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SAMPLER_HPP
