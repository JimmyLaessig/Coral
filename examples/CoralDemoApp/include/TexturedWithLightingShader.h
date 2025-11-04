#include <Coral/Coral.hpp>
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

	void main() override
	{
		csl::InAttribute<csl::float3, "Position"> inPosition;
		csl::InAttribute<csl::float3, "Normal"> inNormal;
		csl::InAttribute<csl::float2, "Texcoord0"> inTexcoord0;

		csl::Parameter<csl::Float4x4, "modelViewProjectionMatrix"> modelViewProjectionMatrix;
		csl::Parameter<csl::Float3x3, "normalMatrix"> normalMatrix;

		csl::OutAttribute<csl::float4, csl::DefaultAttributes::POSITION> outPosition;
		csl::OutAttribute<csl::float3, "WorldNormal"> outWorldNormal;
		csl::OutAttribute<csl::float2, "Texcoord0"> outTexcoord0;


		auto p = modelViewProjectionMatrix * csl::float4(inPosition, 1.f);
		outPosition    = p;
		outWorldNormal = normalMatrix * inNormal;
		outTexcoord0   = inTexcoord0;
	}

}; // class VertexShader


class FragmentShader : public csl::ShaderModule
{
public:

	void main() override
	{
		csl::InAttribute<csl::float3, "WorldNormal"> inWorldNormal;
		csl::InAttribute<csl::float2, "Texcoord0"> inTexcoord0;

		csl::Parameter<csl::float3, "lightColor"> lightColor;
		csl::Parameter<csl::float3, "lightDirection"> lightDirection;
		csl::Parameter<csl::Sampler2D, "colorTexture"> colorTexture;

		csl::OutAttribute<csl::float4, "Color"> outColor;

		//auto worldNormal = csl::normalize(inWorldNormal);
		//auto n_dot_l     = csl::dot(normalize(lightDirection), worldNormal);
		//auto color       = colorTexture.sample(inTexcoord0).xyz();
		//outColor         = csl::float4(color * lightColor * n_dot_l, 1.f);
	}

}; // class FragmentShader

inline Coral::UniformBlockDefinition
uniformBlockDefinition()
{
	return Coral::UniformBlockDefinition({
		{ Coral::UniformFormat::MAT44F, "modelViewProjectionMatrix",  1 },
		{ Coral::UniformFormat::MAT33F, "normalMatrix",  1 },
		{ Coral::UniformFormat::VEC3F, "lightColor", 1 },
		{ Coral::UniformFormat::VEC3F, "lightDirection",  1 },
	});
}


inline std::optional<Coral::ShaderLanguage::Compiler::Result>
shaderSource()
{
	VertexShader vertexShader;
	vertexShader.buildInstructionList();
	FragmentShader fragmentShader;
	fragmentShader.buildInstructionList();
	Coral::ShaderLanguage::CompilerSPV compiler;
	compiler.addShaderModule(Coral::ShaderStage::VERTEX, vertexShader)
		    .addShaderModule(Coral::ShaderStage::FRAGMENT, fragmentShader)
		    .addUniformBlockOverride(0, "Uniforms", uniformBlockDefinition())
		    .addInputAttributeBindingLocation(POSITION_LOCATION, POSITION)
		    .addInputAttributeBindingLocation(NORMAL_LOCATION, NORMAL)
		    .addInputAttributeBindingLocation(TEXCOORD0_LOCATION, TEXCOORD0)
		    .addOutputAttributeBindingLocation(12, "WorldNormal");

	auto result = compiler.compile2();
	if (result)
	{
		std::cout << "-------------------- Vertex shader --------------------" << std::endl;
		std::cout << result->glslResult.vertexShader << std::endl;

		std::cout << "------------------- Fragment shader -------------------" << std::endl;
		std::cout << result->glslResult.fragmentShader << std::endl;

		return { std::move(result->spvResult) };
	}
	else
	{
		std::cerr << result.error().message << std::endl;
		return std::nullopt;
	}
}

} // TexturedWithLightingShader
