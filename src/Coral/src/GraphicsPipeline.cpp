#include <Coral/GraphicsPipeline.h>

#include "GraphicsPipeline.hpp"
#include "Context.hpp"
#include "ShaderModule.hpp"

#include <span>

using namespace Coral;

namespace
{

void
destroy(const CoDescriptorSetLayout& set)
{
    delete[] set.pBindings;
}

void
create(const Coral::BindDescriptorInfo& source, CoBindDescriptorInfo& target)
{
    target.type    = source.type;
    target.binding = source.binding;
    target.count   = source.count;
}

void
create(const Coral::DescriptorSetLayout& source, CoDescriptorSetLayout& target)
{
    auto bindings = new CoBindDescriptorInfo[source.bindings.size()];
    target.pBindings     = bindings;
    target.bindingsCount = source.bindings.size();

    for (size_t i = 0; i < source.bindings.size(); ++i)
    {
        ::create(source.bindings[i], bindings[i]);
    }
}

} // namespace

CoResult
coContextCreateGraphicsPipeline(CoContext context, const CoGraphicsPipelineCreateConfig* pConfig, CoGraphicsPipeline* pPipeline)
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
        *pPipeline = new CoGraphicsPipeline_T{ impl.value() };
        return CO_SUCCESS;
    }
    else
    {
        return static_cast<CoResult>(impl.error());
    }
}


void
coDestroyGraphicsPipeline(CoGraphicsPipeline pipeline)
{
    delete pipeline;
}


CoGraphicsPipeline_T::CoGraphicsPipeline_T(std::shared_ptr<Coral::GraphicsPipeline> impl)
    : impl(impl)
{
    const auto& layouts = impl->getDescriptorSetLayouts();
    mDescriptorSetLayouts.resize(layouts.size());

    for (size_t i = 0; i < layouts.size(); ++i)
    {
        auto& source = layouts[i];
        auto& target = mDescriptorSetLayouts[i];
       
        create(source, target);

    }
}

CoGraphicsPipeline_T::~CoGraphicsPipeline_T()
{
    for (const auto& set : mDescriptorSetLayouts)
    {
        destroy(set);
    }
}


uint32_t
coGraphicsPipelineGetDescriptorSetLayoutCount(const CoGraphicsPipeline pipeline)
{
    return static_cast<uint32_t>(pipeline->mDescriptorSetLayouts.size());
}


void
coGraphicsPipelineGetDescriptorSetLayouts(const CoGraphicsPipeline pipeline, CoDescriptorSetLayout* pDescriptorSetLayouts)
{
    std::copy(pipeline->mDescriptorSetLayouts.begin(), 
              pipeline->mDescriptorSetLayouts.end(),
              pDescriptorSetLayouts);
}
