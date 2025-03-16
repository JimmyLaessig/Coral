#include <Coral/Coral.hpp>
#include <Coral/UniformBlockBuilder.hpp>
#include <Coral/ShaderLanguage.hpp>
#include <Coral/ShaderGraph/CompilerSPV.hpp>

namespace TexturedWithLightingShader
{

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


inline std::optional<Coral::ShaderGraph::Compiler::Result>
shaderSource()
{
	csl::Shader vertexShader;
	csl::Shader fragmentShader;
	// VertexShader
	{
		auto p  = csl::Attribute<csl::Float3>("Position");
		auto n  = csl::Attribute<csl::Float3>("Normal");
		auto uv = csl::Attribute<csl::Float2>("Texcoord0");

		auto modelViewProjectionMatrix = csl::Parameter<csl::Float4x4>("modelViewProjectionMatrix");
		auto normalMatrix = csl::Parameter<csl::Float3x3>("normalMatrix");

		auto position    = modelViewProjectionMatrix * csl::Float4(p, 1.f);
		auto worldNormal = normalMatrix * n;

		vertexShader.addOutput(csl::DefaultSemantics::POSITION, position);
		vertexShader.addOutput("WorldNormal", worldNormal);
		vertexShader.addOutput("Texcoord0", uv);
	}
	// FragmentShader
	{
		auto wn = csl::normalize(csl::Attribute<csl::Float3>("WorldNormal"));
		auto uv = csl::Attribute<csl::Float2>("Texcoord0");

		auto lightColor     = csl::Parameter<csl::Float3>("lightColor");
		auto lightDirection = csl::Parameter<csl::Float3>("lightDirection");
		auto colorTexture   = csl::Parameter<csl::Sampler2D>("colorTexture");

		auto color = colorTexture.sample(uv).xyz();
		color	   = color * lightColor * csl::dot(normalize(lightDirection), wn);
		
		fragmentShader.addOutput("Color", csl::Float4(color, 1.f));
	}

	return Coral::ShaderGraph::CompilerSPV()
		.addShader(Coral::ShaderStage::VERTEX, vertexShader.shaderGraph())
		.addShader(Coral::ShaderStage::FRAGMENT, fragmentShader.shaderGraph())
		.addUniformBlockOverride(0, 0, "Uniforms", uniformBlockDefinition())
		.compile();
}

} // TexturedWithLightingShader
