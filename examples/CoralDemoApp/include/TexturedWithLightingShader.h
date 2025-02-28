#include <Coral/Coral.hpp>
#include <Coral/UniformBlockBuilder.hpp>
#include <Coral/ShaderLanguage.hpp>
#include <Coral/ShaderGraph/CompilerSPV.hpp>

namespace TexturedWithLightingShader
{

inline Coral::UniformBlockDefinition
uniformBlockDefinition()
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
shaderSource()
{
	Coral::ShaderLanguage::ShaderGraph::ShaderProgram progam;
	// VertexShader
	{
		auto p  = csl::Attribute<csl::Float3>(csl::DefaultSemantics::POSITION);
		auto n  = csl::Attribute<csl::Float3>(csl::DefaultSemantics::NORMAL);
		auto uv = csl::Attribute<csl::Float2>(csl::DefaultSemantics::TEXCOORD0);

		auto modelViewProjectionMatrix = csl::Parameter<csl::Float4x4>("modelViewProjectionMatrix");
		auto normalMatrix = csl::Parameter<csl::Float3x3>("normalMatrix");

		auto position    = modelViewProjectionMatrix * csl::Float4(p, 1.f);
		auto worldNormal = normalMatrix * n;

		progam.addVertexShaderOutput(csl::DefaultSemantics::POSITION, position.source());
		progam.addVertexShaderOutput("WorldNormal", worldNormal.source());
		progam.addVertexShaderOutput("Texcoord0", uv.source());
	}
	// FragmentShader
	{
		auto wn = csl::Attribute<csl::Float3>("WorldNormal");
		auto uv = csl::Attribute<csl::Float2>("Texcoord0");

		auto lightColor     = csl::Parameter<csl::Float3>("lightColor");
		auto lightDirection = csl::Parameter<csl::Float3>("lightDirection");
		auto colorTexture   = csl::Parameter<csl::Sampler2D>("colorTexture");

		auto color = colorTexture.sample(uv).xyz();
		color = color * lightColor * csl::dot(normalize(lightDirection), normalize(wn));

		progam.addFragmentShaderOutput("Color", csl::Float4(color, 1.f).source());
	}

	return Coral::ShaderLanguage::ShaderGraph::CompilerSPV()
		.setShaderProgram(progam)
		.addUniformBlockOverride(uniformBlockDefinition())
		.compile();
}

} // TexturedWithLightingShader
