#ifndef SHADERCOMPILER_HEADERGENERATOR_H_
#define SHADERCOMPILER_HEADERGENERATOR_H_

#include <Coral/ShaderCompiler.h>

#include <optional>
#include <string>

namespace Coral
{

CORALSHADERCOMPILER_API std::string buildShaderHeader(std::string_view className,
												      const std::optional<std::string_view>& classNamespace,
												      const Coral::ShaderModuleCompileInfo& compileInfo,
												      const Coral::CompiledShaderModule& shaderProgram);

} // namespace Coral

#endif // !SHADERCOMPILER_HEADERGENERATOR_H_