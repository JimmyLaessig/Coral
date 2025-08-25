#include <Coral/Coral.hpp>
#include <Coral/UniformBlockBuilder.hpp>
#include <Coral/ShaderLanguage.hpp>

#include <iostream>

namespace TexturedWithLightingShader
{

constexpr const uint32_t POSITION_LOCATION  = 1;
constexpr const uint32_t NORMAL_LOCATION    = 0;
constexpr const uint32_t TEXCOORD0_LOCATION = 2;

constexpr const auto POSITION  = "Position";
constexpr const auto NORMAL    = "Normal";
constexpr const auto TEXCOORD0 = "Texcoord0";

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
	Coral::ShaderLanguage::ShaderModule vertexShader;
	Coral::ShaderLanguage::ShaderModule fragmentShader;
	// VertexShader
	{
		csl::Input<csl::float3> p(POSITION);
		csl::Input<csl::float3> n(NORMAL);
		csl::Input<csl::float2> uv(TEXCOORD0);

		csl::Parameter<csl::Float4x4> modelViewProjectionMatrix("modelViewProjectionMatrix");
		csl::Parameter<csl::Float3x3> normalMatrix("normalMatrix");

		auto position    = modelViewProjectionMatrix * csl::float4(p, 1.f);
		auto worldNormal = normalMatrix * n;

		csl::Output outPosition(csl::DefaultAttribute::POSITION, position);
		csl::Output outWorldNormal("WorldNormal", worldNormal);
		csl::Output outTexcoord0("Texcoord0", uv);

		vertexShader.registerOutputAttribute(outPosition);
		vertexShader.registerOutputAttribute(outWorldNormal);
		vertexShader.registerOutputAttribute(outTexcoord0);
	}
	// FragmentShader
	{
		csl::Input<csl::float3> worldNormal("WorldNormal");
		csl::Input<csl::float2> uv("Texcoord0");

		auto wn = csl::normalize(worldNormal);

		csl::Parameter<csl::float3> lightColor("lightColor");
		csl::Parameter<csl::float3> lightDirection("lightDirection");
		csl::Parameter<csl::Sampler2D> colorTexture("colorTexture");

		auto n_dot_l = csl::dot(normalize(lightDirection), worldNormal);
		auto color   = colorTexture.sample(uv).xyz();
		color	     = color * lightColor * n_dot_l;

		csl::Output outColor("Color", csl::float4(color, 1.f));

		fragmentShader.registerOutputAttribute(outColor);
	}

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
