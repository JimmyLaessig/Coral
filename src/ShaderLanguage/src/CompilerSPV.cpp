#include <Coral/ShaderGraph/CompilerSPV.hpp>

#include <shaderc/shaderc.h>

#include <iostream>

using namespace Coral::ShaderLanguage::ShaderGraph;


Compiler&
CompilerSPV::setShaderProgram(const ShaderProgram& shaderProgram)
{
	mCompilerGLSL.setShaderProgram(shaderProgram);
	return *this;
}


Compiler&
CompilerSPV::addUniformBlockOverride(const UniformBlockDefinition& uniformBlock)
{
	mCompilerGLSL.addUniformBlockOverride(uniformBlock);
	return *this;
}


Compiler&
CompilerSPV::setDefaultUniformBlockName(std::string_view name)
{
	mCompilerGLSL.setDefaultUniformBlockName(name);
	return *this;
}


std::optional<CompilerResult>
CompilerSPV::compile()
{
	auto result = mCompilerGLSL.compile();
	if (!result)
	{
		return {};
	}

	for (auto [source, shaderKind] : { std::pair{ &result->vertexShader, shaderc_vertex_shader },
								       std::pair{ &result->fragmentShader, shaderc_fragment_shader } })
	{
		auto options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
		shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_3);

		auto compiler = shaderc_compiler_initialize();

		auto result = shaderc_compile_into_spv(compiler,
											   source->c_str(),
											   source->size(),
											   shaderKind,
											   "shader.glsl",
											   "main",
											   options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
		{
			std::cerr << "Failed to compile shader to SpirV: " << shaderc_result_get_error_message(result) << std::endl;
			return {};
		}

		auto bytes = shaderc_result_get_bytes(result);
		auto size = shaderc_result_get_length(result);

		source->assign(bytes, bytes + size);
	}

	return result;
}
