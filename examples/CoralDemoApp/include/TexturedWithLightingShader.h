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


inline std::optional<Coral::ShaderGraph::CompilerResult>
shaderSource()
{
	csl::ShaderProgram shader;
	// VertexShader
	{
		auto p  = csl::Attribute<csl::Float3>(csl::DefaultSemantics::Position);
		auto n  = csl::Attribute<csl::Float3>(csl::DefaultSemantics::Normal);
		auto uv = csl::Attribute<csl::Float2>(csl::DefaultSemantics::Texcoord0);

		auto modelViewProjectionMatrix = csl::Parameter<csl::Float4x4>("modelViewProjectionMatrix");
		auto normalMatrix = csl::Parameter<csl::Float3x3>("normalMatrix");

		auto position    = modelViewProjectionMatrix * csl::Float4(p, 1.f);
		auto worldNormal = normalMatrix * n;

		shader.addVertexShaderOutput(csl::DefaultSemantics::Position, position);
		shader.addVertexShaderOutput("WorldNormal", worldNormal);
		shader.addVertexShaderOutput("Texcoord0", uv);
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

		shader.addFragmentShaderOutput("Color", csl::Float4(color, 1.f));
	}

	return Coral::ShaderGraph::CompilerSPV()
		.setShaderProgram(shader.getShaderGraphProgram())
		.addUniformBlockOverride(0, 0, "Uniforms", uniformBlockDefinition())
		.compile();
}

} // TexturedWithLightingShader
