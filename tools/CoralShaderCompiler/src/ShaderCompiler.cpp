#include <Coral/ShaderCompiler.h>

#include <Coral/ShaderIncluder.h>

#include <shaderc/shaderc.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace
{

shaderc_shader_kind
convert(Coral::ShaderStage type)
{
	switch (type)
	{
		case Coral::ShaderStage::VERTEX:				return shaderc_vertex_shader;
		case Coral::ShaderStage::FRAGMENT:				return shaderc_fragment_shader;
		case Coral::ShaderStage::GEOMETRY:				return shaderc_glsl_fragment_shader;
		case Coral::ShaderStage::COMPUTE:				return shaderc_compute_shader;
		case Coral::ShaderStage::TESSELLATION_CONTROL:	return shaderc_tess_control_shader;
		case Coral::ShaderStage::TESSELLATION_EVAL:		return shaderc_tess_evaluation_shader;
	}

	std::unreachable();
}

shaderc_source_language
convert(Coral::ShaderLanguage language)
{
	switch (language)
	{
		case Coral::ShaderLanguage::HLSL:	return shaderc_source_language_hlsl;
		case Coral::ShaderLanguage::GLSL:	return shaderc_source_language_glsl;

	}
	std::unreachable();
}

} // namespace


namespace Coral
{

std::optional<CompiledShaderModule>
compileHLSLToSpirV(const ShaderModuleCompileInfo& compileInfo)
{
	auto preprocessedShaderSource = resolveIncludes(compileInfo);

	if (!preprocessedShaderSource)
	{
		return {};
	}

	CompiledShaderModule compiledShader;
	compiledShader.sourceCodePreprocessed = std::move(*preprocessedShaderSource);

	//auto defines = getPreprocessorDefinitions(program);

	auto options = shaderc_compile_options_initialize();
	shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
	shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_3);

	auto compiler = shaderc_compiler_initialize();
	
	auto result = shaderc_compile_into_spv(compiler,
										   compiledShader.sourceCodePreprocessed.c_str(),
										   compiledShader.sourceCodePreprocessed.size(),
										   convert(compileInfo.type),
										   compileInfo.filePath.string().c_str(),
										   compileInfo.entryPoint.c_str(),
										   options);

	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
	{
		std::cerr << "Failed to compile shader to SpirV: " << shaderc_result_get_error_message(result) << std::endl;
		return {};
	}

	auto bytes = shaderc_result_get_bytes(result);
	auto size = shaderc_result_get_length(result);

	compiledShader.spirVCode.resize(size / sizeof(uint32_t));
	std::memcpy(compiledShader.spirVCode.data(), bytes, size);

	result = shaderc_compile_into_spv_assembly(compiler,
											   compiledShader.sourceCodePreprocessed.c_str(),
											   compiledShader.sourceCodePreprocessed.size(),
									           convert(compileInfo.type),
									           compileInfo.filePath.string().c_str(),
									           compileInfo.entryPoint.c_str(),
									           options);

	if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success)
	{
		std::cerr << "Failed to compile shader to SpirV: " << shaderc_result_get_error_message(result) << std::endl;
		return {};
	}

	bytes = shaderc_result_get_bytes(result);
	size = shaderc_result_get_length(result);

	compiledShader.spirVCodeAssembly.assign(bytes, size);

	return { std::move(compiledShader) };
}

} // namespace Coral