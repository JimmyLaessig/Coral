#ifndef SHADERCOMPILER_SHADERINCLUDER_H_
#define SHADERCOMPILER_SHADERINCLUDER_H_

#include <Coral/ShaderCompiler.h>

namespace Coral
{

std::optional<std::string> resolveIncludes(const ShaderModuleCompileInfo& compileInfo);

} // namespace ShaderCompiler

#endif // !SHADERCOMPILER_SHADERINCLUDER_H_