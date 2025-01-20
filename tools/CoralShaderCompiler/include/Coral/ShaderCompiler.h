#ifndef CORAL_SHADERCOMPILER_H_
#define CORAL_SHADERCOMPILER_H_

#include <CoralShaderCompiler/System.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace Coral
{

enum class ShaderStage
{
	VERTEX,
	GEOMETRY,
	FRAGMENT,
	COMPUTE,
	TESSELLATION_CONTROL,
	TESSELLATION_EVAL,
};


enum class ShaderLanguage
{
	HLSL,
	GLSL,
};


struct CORALSHADERCOMPILER_API ShaderModuleCompileInfo
{
	std::string sourceCode;
	std::filesystem::path filePath;
	std::string entryPoint;
	ShaderStage type;
	std::vector<std::filesystem::path> additionalIncludeDirectories;
	ShaderLanguage language;
};


struct CORALSHADERCOMPILER_API CompiledShaderModule
{
	std::string sourceCodePreprocessed;
	std::vector<uint32_t> spirVCode;
	std::string spirVCodeAssembly;
};


CORALSHADERCOMPILER_API std::optional<CompiledShaderModule> compileHLSLToSpirV(const ShaderModuleCompileInfo& program);

} // namespace Coral


#endif // !CORAL_SHADERCOMPILER_H_