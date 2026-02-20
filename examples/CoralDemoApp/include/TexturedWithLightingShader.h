#include <Coral/Coral.h>
#include <Coral/UniformBlockBuilder.hpp>
#include <Coral/ShaderLanguage.hpp>

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

class VertexShader : public csl::ShaderModule
{
public:
	VertexShader() = default;

	csl::Attribute<csl::float3, csl::Location{ 0 }, csl::Qualifier::IN, "Position"> inPosition;
	csl::Attribute<csl::float3, csl::Location{ 1 }, csl::Qualifier::IN, "Normal"> inNormal;
	csl::Attribute<csl::float2, csl::Location{ 2 }, csl::Qualifier::IN, "Texcoord0"> inTexcoord0;

	csl::Attribute<csl::float3, csl::Location{ 0 }, csl::Qualifier::OUT, csl::DefaultAttributes::POSITION> outPosition;
	csl::Attribute<csl::float3, csl::Location{ 0 }, csl::Qualifier::OUT, "OutNormal"> outWorldNormal;
	csl::Attribute<csl::float2, csl::Location{ 1 }, csl::Qualifier::OUT, "OutTexcoord0"> outTexcoord0;

	struct Uniforms
	{
		csl::Float4x4 modelViewProjectionMatrix;
		csl::Float3x3 normalMatrix;
	};

	csl::UniformBuffer<Uniforms, csl::Location{ 0 }> uniforms;

	void main() override
	{
		outPosition    = (uniforms->modelViewProjectionMatrix * csl::float4(*inPosition, 1.f)).xyz();
		outWorldNormal = uniforms->normalMatrix * *inNormal;
		outTexcoord0   = inTexcoord0;
	}

}; // class VertexShader


class FragmentShader : public csl::ShaderModule
{
public:
	FragmentShader() = default;

	using csl::ShaderModule::ShaderModule;

	csl::Attribute<csl::float3, csl::Location{ 0 }, csl::Qualifier::IN, "Normal"> inWorldNormal;
	csl::Attribute<csl::float2, csl::Location{ 1 }, csl::Qualifier::IN, "Texcoord0" > inTexcoord0;

	csl::Attribute<csl::float4, csl::Location{ 0 }, csl::Qualifier::OUT, "Color">  outColor;

	csl::Sampler2D colorTexture;

	struct Uniforms
	{
		csl::float3 lightColor;
		csl::float3 lightDirection;
	};

	csl::UniformBuffer<Uniforms, csl::Location{ 1 }> uniforms;

	void main() override
	{
		auto worldNormal = csl::normalize(*inWorldNormal);
		auto n_dot_l     = csl::dot(normalize(uniforms->lightDirection), worldNormal);
		auto color       = colorTexture.sample(*inTexcoord0).xyz();
		outColor         = csl::float4(color * uniforms->lightColor * n_dot_l * 0.5f, 1.f);
	}

}; // class FragmentShader

//inline Coral::UniformBlockDefinition
//uniformBlockDefinition()
//{
//	return Coral::UniformBlockDefinition({
//		{ Coral::UniformFormat::MAT44F, "modelViewProjectionMatrix",  1 },
//		{ Coral::UniformFormat::MAT33F, "normalMatrix",               1 },
//		{ Coral::UniformFormat::VEC3F,  "lightColor",                 1 },
//		{ Coral::UniformFormat::VEC3F,  "lightDirection",             1 },
//	});
//}


inline std::optional<Coral::ShaderLanguage::Compiler::Result>
shaderSource()
{
	VertexShader vertexShader;
	vertexShader.buildInstructionList();
	FragmentShader fragmentShader;
	fragmentShader.buildInstructionList();
	Coral::ShaderLanguage::CompilerSPV compiler;
	auto result = compiler.Compile(vertexShader, fragmentShader);
	if (result)
	{
		auto glslResult = compiler.GetCompiledShaderSourceGLSL();

		std::cout << "-------------------- Vertex shader --------------------" << std::endl;
		std::cout << glslResult.vertexShader << std::endl;

		std::cout << "------------------- Fragment shader -------------------" << std::endl;
		std::cout << glslResult.fragmentShader << std::endl;

		return { std::move(result.value()) };
	}
	else
	{
		std::cerr << result.error().message << std::endl;
		return std::nullopt;
	}
}

} // TexturedWithLightingShader
