#include <Coral/GraphicsPipeline.h>

#include "GraphicsPipeline.hpp"
#include "Context.hpp"
#include "ShaderModule.hpp"

#include <span>

using namespace Coral;


CoResult
coContextCreateGraphicsPipeline(CoContext context, const CoGraphicsPipelineCreateConfig* pConfig, CoGraphicsPipeline* pGraphicsPipeline)
{
    GraphicsPipeline::CreateConfig configImpl;
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

    if (auto impl = context->impl->createGraphicsPipeline(configImpl))
    {
        *pGraphicsPipeline = new CoGraphicsPipeline_T{ impl.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroyGraphicsPipeline(CoGraphicsPipeline GraphicsPipeline)
{
    delete GraphicsPipeline;
}
