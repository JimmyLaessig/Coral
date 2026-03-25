
#include <Coral/Util/UniformBlockBuilder.hpp>
#include <csl/csl.hpp>

#include <iostream>
#include <string_view>

namespace TexturedWithLightingShader
{

constexpr const uint32_t POSITION_LOCATION  = 0;
constexpr const uint32_t NORMAL_LOCATION    = 1;
constexpr const uint32_t TEXCOORD0_LOCATION = 2;

constexpr const auto POSITION  = "Position";
constexpr const auto NORMAL    = "Normal";
constexpr const auto TEXCOORD0 = "Texcoord0";
constexpr const auto COLOR     = "Color";

struct VertexInput
{
	csl::Attribute<csl::float3, "Position", csl::Location{ 0 }, csl::Qualifier::IN> position;
	csl::Attribute<csl::float3, "Normal", csl::Location{ 1 }, csl::Qualifier::IN> normal;
	csl::Attribute<csl::float2, "Texcoord0", csl::Location{ 2 }, csl::Qualifier::IN> texcoord0;
};


struct VertexOutput
{
	csl::Attribute<csl::float4, csl::SV_POSITION> position;
	csl::Attribute<csl::float3, "Normal", csl::Location{ 0 }, csl::Qualifier::OUT> worldNormal;
	csl::Attribute<csl::float2, "Texcoord0", csl::Location{ 1 }, csl::Qualifier::OUT> texcoord0;
};


struct Uniforms
{
	csl::Float4x4 modelViewProjectionMatrix;
	csl::Float3x3 normalMatrix;
	csl::float3 lightColor;
	csl::float3 lightDirection;
};


void DefineType(Uniforms& uniforms, csl::UniformBufferBase& uniformBuffer)
{
	uniformBuffer.RegisterMember<"modelViewProjectionMatrix">(uniforms.modelViewProjectionMatrix);
	uniformBuffer.RegisterMember<"normalMatrix">(uniforms.normalMatrix);
	uniformBuffer.RegisterMember<"lightColor">(uniforms.lightColor);
	uniformBuffer.RegisterMember<"lightDirection">(uniforms.lightDirection);
}


struct FragmentInput
{
	csl::Attribute<csl::float3, csl::StringLiteral{ "worldNormal" }, csl::Location{ 0 }, csl::Qualifier::IN> worldNormal;
	csl::Attribute<csl::float2, "texcoord0", csl::Location{ 1 }, csl::Qualifier::IN > texcoord0;
};


struct FragmentOutput
{
	csl::Attribute<csl::float4, "color", csl::Location{ 0 }, csl::Qualifier::OUT> color;
};


struct VertexShader
{
	VertexOutput main(const VertexInput& input, 
		              const csl::UniformBuffer<Uniforms, "Uniforms", csl::Location{ 0 }>& uniforms)
	{
		VertexOutput output;
		output.position    = uniforms->modelViewProjectionMatrix * csl::float4(*input.position, 1.f);
		output.worldNormal = csl::normalize(uniforms->normalMatrix * (*input.normal));
		output.texcoord0   = input.texcoord0;
		return output;
	}
};


struct FragmentShader
{
	FragmentOutput main(const FragmentInput& input,
		                const csl::UniformBuffer<Uniforms, "Uniforms", csl::Location{ 0 }>& uniforms,
		                const csl::Sampler2D<"colorTexture", csl::Location{ 1 } >& colorTexture)
	{
		FragmentOutput output;
		auto worldNormal = csl::normalize(*input.worldNormal);
		auto lightDir    = uniforms->lightDirection;
		auto n_dot_l     = csl::dot(lightDir, worldNormal);
		auto color       = colorTexture.sample(*input.texcoord0).xyz();
		output.color     = csl::float4(color * uniforms->lightColor * n_dot_l, 1.f);
		return output;
	}
};

VertexOutput vertexShaderMain(const VertexInput& input, 
	                          const csl::UniformBuffer<Uniforms, "Uniforms", csl::Location{ 0 }>& uniforms)
{
	VertexOutput output;
	output.position    = uniforms->modelViewProjectionMatrix * csl::float4(*input.position, 1.f);
	output.worldNormal = csl::normalize(uniforms->normalMatrix * (*input.normal));
	output.texcoord0   = input.texcoord0;

	return output;
}


FragmentOutput fragmentShaderMain(const FragmentInput& input, 
	                              const csl::UniformBuffer<Uniforms, "Uniforms", csl::Location{ 0 }>& uniforms,
	const csl::Sampler2D < "colorTexture", csl::Location{ 1 } > & colorTexture)
{
	FragmentOutput output;

	auto worldNormal = csl::normalize(*input.worldNormal);
	auto lightDir    = uniforms->lightDirection;
	auto n_dot_l     = csl::dot(lightDir, worldNormal);
	auto color       = colorTexture.sample(*input.texcoord0).xyz();
	output.color     = csl::float4(color * uniforms->lightColor *  n_dot_l, 1.f);
	return output;
}


struct ShaderSource
{
	std::string vertexShader;
	std::string fragmentShader;
};


inline std::optional<ShaderSource>
shaderSource()
{
	ShaderSource shaderSource;

	{
		csl::ShaderGraph shaderGraph(VertexShader{});


		csl::CompilerSPV compiler;
		auto result = compiler.Compile(shaderGraph, csl::ShaderStage::VERTEX);

		std::cout << "-------------------- Vertex shader --------------------" << std::endl;
		std::cout << compiler.GetShaderSourceGLSL().shaderCode << std::endl;

		if (!result)
		{
			std::cerr << result.error().message << std::endl;
			return {};
		}
		else
		{
			shaderSource.vertexShader = std::move(result->shaderCode);
		}
	}

	{
		csl::ShaderGraph shaderGraph(FragmentShader{});
		csl::CompilerSPV compiler;
		auto result = compiler.Compile(shaderGraph, csl::ShaderStage::FRAGMENT);

		std::cout << "-------------------- Fragment shader --------------------" << std::endl;
		std::cout << compiler.GetShaderSourceGLSL().shaderCode << std::endl;

		if (!result)
		{
			std::cerr << result.error().message << std::endl;
			return {};
		}
		else
		{
			shaderSource.fragmentShader = std::move(result->shaderCode);

		}
	}

	return { std::move(shaderSource) };
}

} // TexturedWithLightingShader
