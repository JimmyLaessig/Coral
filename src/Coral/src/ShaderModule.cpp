#include <Coral/ShaderModule.h>

#include "ShaderModule.hpp"
#include "ShaderModuleReflection.hpp"
#include "Context.hpp"

using namespace Coral;

CoShaderModule_T::~CoShaderModule_T()
{
    if (mReflection)
    {
        destroy(*mReflection);
    }
}


ShaderModule::CreateConfig::CreateConfig(const CoShaderModuleCreateConfig& config)
    : name(config.pName ? config.pName : "")
    , stage(config.stage)
    , source(std::span<const std::byte>(reinterpret_cast<const std::byte*>(config.pSource), config.sourceCount))
    , entryPoint(config.pEntryPoint ? config.pEntryPoint : "")
{
}


CoResult
coContextCreateShaderModule(CoContext context,
                            const CoShaderModuleCreateConfig* pConfig,
                            CoShaderModule* pShaderModule)
{
    auto impl = context->impl->createShaderModule({ *pConfig });
    if (impl)
    {
        *pShaderModule = new CoShaderModule_T{ impl.value() };
        return CO_SUCCESS;
    }

    return static_cast<CoResult>(impl.error());
}


void
coDestroyShaderModule(CoShaderModule shaderModule)
{
    delete shaderModule;
}
