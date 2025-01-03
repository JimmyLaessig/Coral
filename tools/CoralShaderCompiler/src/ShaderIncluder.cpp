#include <Coral/ShaderIncluder.h>

#include <fstream>
#include <iostream>
#include <cassert>

#include <shaderc/shaderc.h>

namespace
{

struct ResultUserData
{
	std::string content;
	std::string path;
};

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
		default:
			assert(false);
			return {};
	}
}

} // namespace


class ShaderIncluder
{
public:

	ShaderIncluder(const std::vector<std::filesystem::path>& additionalIncludeDirectories)
		: mAdditionalIncludeDirectories(additionalIncludeDirectories)
	{}

	
	std::optional<std::string> resolveIncludes(const std::string& content,
											   const std::filesystem::path& filePath,
											   Coral::ShaderStage type,
											   const std::string& entryPoint)
	{
		auto options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, 0);
		shaderc_compile_options_set_include_callbacks(options, &resolveInclude, &releaseInclude, (void*)this);

		auto compiler = shaderc_compiler_initialize();

		auto result = shaderc_compile_into_preprocessed_text(compiler,
			content.c_str(),
			content.size(),
			convert(type),
			filePath.string().c_str(),
			entryPoint.c_str(),
			options);

		std::optional<std::string> res;
		auto err = shaderc_result_get_compilation_status(result);
		if (err != shaderc_compilation_status_success)
		{
			std::cerr << "Failed to preprocess shader '" << filePath << "': " << shaderc_result_get_error_message(result) << std::endl;
		}
		else
		{
			res = std::string(shaderc_result_get_bytes(result), shaderc_result_get_length(result));
		}

		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		return res;
	}

private:

	static shaderc_include_result* resolveInclude(void* userData,
		const char* requestedSource,
		int type,
		const char* requestingSource,
		size_t includeDepth)
	{
		auto includer = static_cast<ShaderIncluder*>(userData);

		std::vector<std::filesystem::path> candidates;

		std::filesystem::path requesting = requestingSource;
		std::filesystem::path requested = requestedSource;

		if (requested.is_absolute())
		{
			candidates.push_back(requested);
		}
		else
		{
			candidates.push_back(requesting.parent_path() / requested);

			for (const auto& workingDir : includer->mAdditionalIncludeDirectories)
			{
				candidates.push_back(workingDir / requested);
			}
		}

		auto result = new ResultUserData{ };

		for (auto& path : candidates)
		{
			std::ifstream file(path, std::ios::in | std::ios::binary);

			if (file.good())
			{
				std::stringstream content;
				content << file.rdbuf();

				result->content = content.str();
				result->path = path.generic_string();
			}
		}

		if (result->content.empty())
		{
			result->content = std::string("Cannot find file");
		}

		return new shaderc_include_result{ result->path.c_str(),
										   result->path.size(),
										   result->content.c_str(),
										   result->content.size(),
										   result };
	}

	static void releaseInclude(void* userData, shaderc_include_result* includeResult)
	{
		if (includeResult)
		{
			if (includeResult->user_data)
			{
				delete static_cast<ResultUserData*>(includeResult->user_data);
			}

			delete includeResult;
		}
	}

	std::vector<std::filesystem::path> mAdditionalIncludeDirectories;
};

namespace Coral
{

std::optional<std::string> resolveIncludes(const ShaderModuleCompileInfo& compileInfo)
{
	return ShaderIncluder(compileInfo.additionalIncludeDirectories).resolveIncludes(compileInfo.sourceCode, compileInfo.filePath, compileInfo.type, compileInfo.entryPoint);
}

} // namespace ShaderCompiler