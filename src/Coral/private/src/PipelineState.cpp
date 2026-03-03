#include <Coral/PipelineState.h>
#include <Coral/PipelineState.hpp>
#include <Coral/Context.hpp>
#include <Coral/ShaderModule.hpp>

#include <span>

using namespace Coral;


CoResult
coContextCreatePipelineState(CoContext context, const CoPipelineStateCreateConfig* pConfig, CoPipelineState* pPipelineState)
{
    PipelineState::CreateConfig configImpl;
    configImpl.blendMode       = pConfig->blendMode;
    configImpl.depthTestMode   = pConfig->depthTestMode;
    configImpl.faceCullingMode = pConfig->faceCullingMode;
    configImpl.polygonMode     = pConfig->polygonMode;
    configImpl.topology        = pConfig->topology;

    std::vector<ShaderModulePtr> shaderModules;
    if (pConfig->vertexShaderModule)
    {
        shaderModules.push_back(pConfig->vertexShaderModule->impl);
    }
    if (pConfig->fragmentShaderModule)
    {
        shaderModules.push_back(pConfig->fragmentShaderModule->impl);
    }

    configImpl.shaderModules = shaderModules;
    for (auto attachment : std::span(pConfig->framebufferLayout.pColorAttachments, pConfig->framebufferLayout.colorAttachmentCount))
    {
        configImpl.framebufferLayout.colorAttachments.push_back(attachment);
    }

    if (pConfig->framebufferLayout.depthStencilAttachment)
    {
        configImpl.framebufferLayout.depthStencilAttachment = *pConfig->framebufferLayout.depthStencilAttachment;
    }

    if (auto impl = context->impl->createPipelineState(configImpl))
    {
        *pPipelineState = new CoPipelineState_T{ impl.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroyPipelineState(CoPipelineState pipelineState)
{
    delete pipelineState;
}
