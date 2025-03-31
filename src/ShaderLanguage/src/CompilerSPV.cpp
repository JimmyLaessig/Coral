#include <Coral/ShaderGraph/CompilerSPV.hpp>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include <iostream>
#include <sstream>

using namespace Coral::ShaderGraph;


Compiler&
CompilerSPV::addShader(Coral::ShaderStage stage, const Shader& shader)
{
	mCompilerGLSL.addShader(stage, shader);
	return *this;
}


Compiler&
CompilerSPV::addUniformBlockOverride(uint32_t binding, std::string_view name, const UniformBlockDefinition& uniformBlock)
{
	mCompilerGLSL.addUniformBlockOverride(binding, name, uniformBlock);
	return *this;
}


Compiler&
CompilerSPV::setDefaultUniformBlockName(std::string_view name)
{
	mCompilerGLSL.setDefaultUniformBlockName(name);
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


std::optional<Compiler::Result>
CompilerSPV::compile()
{
	auto result = mCompilerGLSL.compile();
	if (!result)
	{
		return {};
	}

	auto shaderDeleter = [](glslang_shader_t* shader) { glslang_shader_delete(shader); };
	using ShaderPtr = std::unique_ptr<glslang_shader_t, decltype(shaderDeleter)>;

	auto programDeleter = [](glslang_program_t* program) { glslang_program_delete(program); };
	using ProgramPtr = std::unique_ptr<glslang_program_t, decltype(programDeleter)>;

	glslang_initialize_process();

	for (auto& [source, stage] : { std::pair{ &result->vertexShader, GLSLANG_STAGE_VERTEX },
								   std::pair{ &result->fragmentShader, GLSLANG_STAGE_FRAGMENT } })
	{
		glslang_input_t input{};
		input.language				            = GLSLANG_SOURCE_GLSL;
		input.stage					            = stage;
		input.client				            = GLSLANG_CLIENT_VULKAN;
		input.client_version		            = GLSLANG_TARGET_VULKAN_1_3;
		input.target_language	                = GLSLANG_TARGET_SPV;
		input.target_language_version           = GLSLANG_TARGET_SPV_1_3;
		input.code					            = source->c_str();
		input.default_version	                = 100;
		input.default_profile		            = GLSLANG_NO_PROFILE;
		input.force_default_version_and_profile = false;
		input.forward_compatible			    = false;
		input.messages                          = GLSLANG_MSG_DEFAULT_BIT;
		input.resource		                    = glslang_default_resource();

		ShaderPtr shader(glslang_shader_create(&input));
		ProgramPtr program(glslang_program_create());

		if (!shader)
		{
			return {};
		}

		if (!program)
		{
			return {};
		}

		if (!glslang_shader_preprocess(shader.get(), &input))
		{
			std::cerr << "Failed to compile shader to SpirV: " << std::endl;
			std::cerr << glslang_shader_get_info_log(shader.get()) << std::endl;
			std::cerr << glslang_shader_get_info_debug_log(shader.get()) << std::endl;
			return {};
		}

		if (!glslang_shader_parse(shader.get(), &input))
		{
			std::cerr << "Failed to compile shader to SpirV: " << std::endl;
			std::cerr << glslang_shader_get_info_log(shader.get()) << std::endl;
			std::cerr << glslang_shader_get_info_debug_log(shader.get()) << std::endl;
			return {};
		}
	
		glslang_program_add_shader(program.get(), shader.get());
	
		if (!glslang_program_link(program.get(), GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
		{
			std::cerr << "Failed to compile shader to SpirV: " << std::endl;
			std::cerr << glslang_program_get_info_log(program.get()) << std::endl;
			std::cerr << glslang_program_get_info_debug_log(program.get()) << std::endl;

			return {};
		}

		glslang_program_SPIRV_generate(program.get(), stage);

		size_t size = glslang_program_SPIRV_get_size(program.get());
		source->resize(size * sizeof(uint32_t));
		glslang_program_SPIRV_get(program.get(), reinterpret_cast<unsigned int*>(source->data()));
	}

	return result;
}
