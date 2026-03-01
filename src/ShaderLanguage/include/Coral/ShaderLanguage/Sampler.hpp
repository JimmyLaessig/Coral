#ifndef CORAL_SHADERLANGUAGE_SAMPLER_HPP
#define CORAL_SHADERLANGUAGE_SAMPLER_HPP

#include <Coral/ShaderLanguage/Vector.hpp>
#include <Coral/ShaderLanguage/Attribute.hpp>

#include <Coral/ShaderLanguage/Expression.hpp>

namespace Coral::ShaderLanguage
{

template<Semantic S, Location L>
struct Sampler2D : Value
{
	Sampler2D()
		: Value(ShaderGraph::PushExpression<SamplerExpression>(static_cast<uint32_t>(L), S))
	{
	}

	static constexpr ValueType toValueType() { return ValueType::SAMPLER2D; }

	template<typename Vec> 
		requires VectorType<Vec, float, 2>
	float4 sample(Vec&& uv) const
	{
		return float4(ShaderGraph::PushExpression<NativeFunctionExpression>(ValueType::FLOAT4,
			                                                                NativeFunction::SAMPLE, 
			                                                                source(), 
			                                                                std::forward<Vec&&>(uv).source()));
	}

	template<typename S>
		requires ScalarType<S, int>
	int2 size(S&& lod) const
	{
		return { ShaderGraph::PushExpression<NativeFunctionExpression>(ValueType::FLOAT3,
			                                                           NativeFunction::TEXTURE_SIZE, 
			                                                           source(), 
			                                                           std::forward<S&&>(lod).source()) };
	}

	Sampler2D(const Sampler2D&) = delete;
	Sampler2D& operator=(const Sampler2D&) = delete;

	Sampler2D(Sampler2D&&) = delete;
	Sampler2D& operator=(Sampler2D&&) = delete;
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SAMPLER_HPP
