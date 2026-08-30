#include <Coral/ShaderModule.h>

#include "ShaderModule.hpp"
#include "Context.hpp"
#include "Visitor.hpp"

using namespace Coral;


CoResult
coContextCreateShaderModule(CoContext context,
                            const CoShaderModuleCreateConfig* pConfig,
                            CoShaderModule* pShaderModule)
{
    auto impl = context->impl->createShaderModule(*pConfig);
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


void
coShaderModuleGetDescriptorLayout(const CoShaderModule shaderModule, CoDescriptorLayout* pLayout)
{
    *pLayout = shaderModule->impl->descriptorLayout();
}


void
coShaderModuleGetAttributeLayout(const CoShaderModule shaderModule, CoAttributeLayout* pLayout)
{
    *pLayout = shaderModule->impl->attributeLayout();
}
