#include <Coral/Coral.hpp>
#include <Coral/UniformBlockBuilder.hpp>
#include <Coral/ShaderLanguage.hpp>
#include <Coral/ShaderGraph/CompilerSPV.hpp>

inline Coral::UniformBlockDefinition
TexturedWithLightingShaderUniformBlockDefinition()
{
	return Coral::UniformBlockDefinition("Uniforms",
	{
		{ Coral::ValueType::MAT44F, "modelViewProjectionMatrix",  1 },
		{ Coral::ValueType::MAT33F, "normalMatrix",  1 },
		{ Coral::ValueType::VEC3F, "lightColor", 1 },
		{ Coral::ValueType::VEC3F, "lightDirection",  1 },
	});
}


inline std::optional<Coral::ShaderLanguage::ShaderGraph::CompilerResult>
TexturedWithLightingShader()
{
	Coral::ShaderLanguage::ShaderGraph::ShaderProgram progam;
	// VertexShader
	{
		auto p = csl::Attribute<csl::Float3>("Position");
		auto n = csl::Attribute<csl::Float3>("Normal");
		auto uv = csl::Attribute<csl::Float2>("Texcoord0");

		auto modelViewProjectionMatrix = csl::Parameter<csl::Float4x4>("modelViewProjectionMatrix");
		auto normalMatrix = csl::Parameter<csl::Float3x3>("normalMatrix");

		auto position = modelViewProjectionMatrix * csl::Float4(p, 1.f);
		auto worldNormal = normalMatrix * n;

		progam.addOutput(Coral::ShaderLanguage::ShaderGraph::ShaderStage::VERTEX, csl::ShaderGraph::DefaultSemantics::POSITION, position.source());
		progam.addOutput(Coral::ShaderLanguage::ShaderGraph::ShaderStage::VERTEX, "WorldNormal", worldNormal.source());
		progam.addOutput(Coral::ShaderLanguage::ShaderGraph::ShaderStage::VERTEX, "Texcoord0", uv.source());
	}
	// FragmentShader
	{
		auto n = csl::Attribute<csl::Float3>("WorldNormal");
		auto uv = csl::Attribute<csl::Float2>("Texcoord0");

		auto lightColor = csl::Parameter<csl::Float3>("lightColor");
		auto lightDirection = csl::Parameter<csl::Float3>("lightDirection");
		auto colorTexture = csl::Parameter<csl::Sampler2D>("colorTexture");

		auto color = colorTexture.sample(uv).xyz();
		color = color * lightColor * csl::dot(normalize(lightDirection), normalize(n));

		progam.addOutput(Coral::ShaderLanguage::ShaderGraph::ShaderStage::FRAGMENT, "Color", csl::Float4(color, 1.f).source());
	}

	return Coral::ShaderLanguage::ShaderGraph::CompilerSPV()
		.setShaderProgram(progam)
		.addUniformBlockOverride(TexturedWithLightingShaderUniformBlockDefinition())
		.compile();
}
