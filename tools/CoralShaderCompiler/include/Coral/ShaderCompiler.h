#ifndef CORAL_SHADERCOMPILER_H_
#define CORAL_SHADERCOMPILER_H_

#include <CoralShaderCompiler/System.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

class GLFWwindow;

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


struct ShaderModuleCompileInfo
{
	std::string sourceCode;
	std::filesystem::path filePath;
	std::string entryPoint;
	ShaderStage type;
	std::vector<std::filesystem::path> additionalIncludeDirectories;
	ShaderLanguage language;
};


struct CompiledShaderModule
{
	std::string sourceCodePreprocessed;
	std::vector<uint32_t> spirVCode;
	std::string spirVCodeAssembly;
};


std::optional<CompiledShaderModule> CORALSHADERCOMPILER_API compileHLSLToSpirV(const ShaderModuleCompileInfo& program);

} // namespace Coral


#endif // !CORAL_SHADERCOMPILER_H_