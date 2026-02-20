#ifndef CORAL_SHADERLANGUAGE_SAMPLER_HPP
#define CORAL_SHADERLANGUAGE_SAMPLER_HPP

#include <Coral/ShaderLanguage/Vector.hpp>
#include <Coral/ShaderLanguage/Expression.hpp>

namespace Coral::ShaderLanguage
{
struct Sampler2D : Value
{
	using Value::Value;

	static constexpr ValueType toValueType() { return ValueType::SAMPLER2D; }

	template<typename VectorType> 
		requires IsVector<VectorType, float, 2>
	float4 sample(VectorType&& uv)
	{
		return float4{ pushExpression<NativeFunctionExpression>(ValueType::FLOAT4,
			                                                    "texture", 
			                                                    source(), 
			                                                    std::forward<VectorType&&>(uv).source()) };
	}

	template<typename ScalarType>
		requires IsScalar<ScalarType, int>
	int2 size(ScalarType&& lod)
	{
		return { pushExpression<NativeFunctionExpression>(ValueType::FLOAT3,
			                                              "textureSize", 
			                                              source(), 
			                                              std::forward<ScalarType&&>(lod).source()) };
	}
};

} // namespace ShaderLanguage 

#endif // !CORAL_SHADERLANGUAGE_SAMPLER_HPP
