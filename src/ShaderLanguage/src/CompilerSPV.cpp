#include <Coral/ShaderGraph/CompilerSPV.hpp>

#include <shaderc/shaderc.h>

#include <iostream>
#include <sstream>

using namespace Coral::ShaderGraph;


Compiler&
CompilerSPV::setShaderProgram(const Program& shaderProgram)
{
	mCompilerGLSL.setShaderProgram(shaderProgram);
	return *this;
}


Compiler&
CompilerSPV::addUniformBlockOverride(uint32_t set, uint32_t binding, std::string_view name, const UniformBlockDefinition& uniformBlock)
{
	mCompilerGLSL.addUniformBlockOverride(set, binding, name, uniformBlock);
	return *this;
}


Compiler&
CompilerSPV::setDefaultUniformBlockName(std::string_view name)
{
	mCompilerGLSL.setDefaultUniformBlockName(name);
	return *this;
}


Compiler&
CompilerSPV::setDefaultDescriptorSet(uint32_t set)
{
	mCompilerGLSL.setDefaultDescriptorSet(set);
	return *this;
}


void
printShaderCode(const std::string& code)
{
	uint32_t lineNumber = 1;

	std::stringstream ss(code);
	std::string line;
	while (std::getline(ss, line, '\n'))
	{
		std::cout << lineNumber++ << ": " << line << std::endl;
	}
}


std::optional<CompilerResult>
CompilerSPV::compile()
{
	auto result = mCompilerGLSL.compile();
	if (!result)
	{
		return {};
	}

	for (auto [source, shaderKind, name] : { std::tuple{ &result->vertexShader, shaderc_vertex_shader, "VertexShader.glsl" },
										     std::tuple{ &result->fragmentShader, shaderc_fragment_shader, "FragmentShader.glsl" } })
	{
		auto options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
		shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
		shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_3);
		shaderc_compile_options_set_warnings_as_errors(options);

		auto compiler = shaderc_compiler_initialize();

		auto result = shaderc_compile_into_spv(compiler,
											   source->c_str(),
											   source->size(),
											   shaderKind,
											   name,
											   "main",
											   options);

		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
		{
			std::cerr << "Failed to compile shader to SpirV: " << shaderc_result_get_error_message(result) << std::endl;
			printShaderCode(*source);
			return {};
		}

		printShaderCode(*source);
		auto bytes = shaderc_result_get_bytes(result);
		auto size = shaderc_result_get_length(result);

		source->assign(bytes, bytes + size);
	}

	return result;
}
