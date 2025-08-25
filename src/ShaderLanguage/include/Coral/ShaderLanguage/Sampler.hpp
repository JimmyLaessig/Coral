#ifndef CORAL_SHADERLANGUAGE_SAMPLER_HPP
#define CORAL_SHADERLANGUAGE_SAMPLER_HPP

#include <Coral/ShaderLanguage/Vector.hpp>

namespace Coral::ShaderLanguage
{
struct Sampler2D : Value
{
	using Value::Value;

	static constexpr ValueType toShaderTypeId() { return ValueType::SAMPLER2D; }

	float4 sample(float2 uv)
	{
		return { std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT4, "texture", source(), uv.source()) };
	}

	int2 size(Int lod)
	{
		return { std::make_shared<ShaderGraph::NativeFunctionExpression>(ValueType::FLOAT3, "textureSize", source(), lod.source()) };
	}
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SAMPLER_HPP
