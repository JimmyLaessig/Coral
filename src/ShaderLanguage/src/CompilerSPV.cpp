#include <Coral/ShaderLanguage/CompilerSPV.hpp>

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

#include <iostream>
#include <sstream>

using namespace Coral::ShaderLanguage;

namespace
{

glslang_stage_t Convert(Coral::ShaderLanguage::ShaderStage stage)
{
	switch (stage)
	{
		case Coral::ShaderLanguage::ShaderStage::VERTEX:   return GLSLANG_STAGE_VERTEX;
		case Coral::ShaderLanguage::ShaderStage::FRAGMENT: return GLSLANG_STAGE_FRAGMENT;
	}

	std::unreachable();
}

} // namespace

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


std::expected<Compiler::Result, Compiler::Error>
CompilerSPV::Compile(const ShaderGraph& shaderModule, ShaderStage shaderStage)
{
	mShaderModule = &shaderModule;
	return mCompilerGLSL.Compile(shaderModule, shaderStage).and_then([this, shaderStage](const Compiler::Result& result) { return CompileSPV(result, shaderStage); });
}


Compiler::Result
CompilerSPV::GetShaderSourceGLSL() const
{
	return mShaderSourceGLSL;
}


std::expected<Compiler::Result, Compiler::Error>
CompilerSPV::CompileSPV(const Compiler::Result& result, ShaderStage shaderStage)
{ 
	mShaderSourceGLSL = result;
	
	auto spvResult = result;

	auto shaderDeleter = [](glslang_shader_t* shader) { glslang_shader_delete(shader); };
	using ShaderPtr = std::unique_ptr<glslang_shader_t, decltype(shaderDeleter)>;

	auto programDeleter = [](glslang_program_t* program) { glslang_program_delete(program); };
	using ProgramPtr = std::unique_ptr<glslang_program_t, decltype(programDeleter)>;

	glslang_initialize_process();

	glslang_input_t input{};
	input.language                          = GLSLANG_SOURCE_GLSL;
	input.stage                             = Convert(shaderStage);
	input.client                            = GLSLANG_CLIENT_VULKAN;
	input.client_version                    = GLSLANG_TARGET_VULKAN_1_3;
	input.target_language                   = GLSLANG_TARGET_SPV;
	input.target_language_version           = GLSLANG_TARGET_SPV_1_3;
	input.code                              = mShaderSourceGLSL.shaderCode.c_str();
	input.default_version                   = 100;
	input.default_profile                   = GLSLANG_NO_PROFILE;
	input.force_default_version_and_profile = false;
	input.forward_compatible                = false;
	input.messages                          = GLSLANG_MSG_DEFAULT_BIT;
	input.resource                          = glslang_default_resource();

	ShaderPtr shader(glslang_shader_create(&input));
	ProgramPtr program(glslang_program_create());

	if (!shader || !program)
	{
		return std::unexpected(Error{ "An unexpected error occurred. "} );
	}

	if (!glslang_shader_preprocess(shader.get(), &input))
	{
		std::stringstream ss;
		ss << "Failed to compile shader to SpirV: " << std::endl;
		ss << glslang_shader_get_info_log(shader.get()) << std::endl;
		ss << glslang_shader_get_info_debug_log(shader.get()) << std::endl;
		return std::unexpected(Error{ ss.str() });
	}

	if (!glslang_shader_parse(shader.get(), &input))
	{
		std::stringstream ss;
		ss << "Failed to compile shader to SpirV: " << std::endl;
		ss << glslang_shader_get_info_log(shader.get()) << std::endl;
		ss << glslang_shader_get_info_debug_log(shader.get()) << std::endl;
		return std::unexpected(Error{ ss.str() });
	}

	glslang_program_add_shader(program.get(), shader.get());

	if (!glslang_program_link(program.get(), GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
	{
		std::stringstream ss;
		ss << "Failed to compile shader to SpirV: " << std::endl;
		ss << glslang_program_get_info_log(program.get()) << std::endl;
		ss << glslang_program_get_info_debug_log(program.get()) << std::endl;

		return std::unexpected(Error{ ss.str() });
	}
	
	glslang_program_SPIRV_generate(program.get(), input.stage);

	size_t size = glslang_program_SPIRV_get_size(program.get());
	spvResult.shaderCode.resize(size * sizeof(uint32_t));
	glslang_program_SPIRV_get(program.get(), reinterpret_cast<unsigned int*>(spvResult.shaderCode.data()));

	return spvResult;
}
