
#include "GraphicsPipelineImpl.hpp"

#include "ContextImpl.hpp"
#include "ShaderModuleImpl.hpp"
#include "VulkanFormat.hpp"

#include "../Finally.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

using namespace Coral::Vulkan;

namespace
{

VkShaderStageFlagBits
convert(CoShaderStage shaderStage)
{
    switch (shaderStage)
    {
        case CO_SHADER_STAGE_VERTEX:   return VK_SHADER_STAGE_VERTEX_BIT;
        case CO_SHADER_STAGE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        default: assert(false);        return VK_SHADER_STAGE_ALL_GRAPHICS;
    }
}


VkFormat
convert(CoAttributeFormat format)
{
    switch (format)
    {
        case CO_ATTRIBUTE_FORMAT_VEC4F:  return VK_FORMAT_R32G32B32A32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_VEC3F:  return VK_FORMAT_R32G32B32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_VEC2F:  return VK_FORMAT_R32G32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_FLOAT:  return VK_FORMAT_R32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_INT32:  return VK_FORMAT_R32_SINT;
        case CO_ATTRIBUTE_FORMAT_INT16:  return VK_FORMAT_R16_SINT;
        case CO_ATTRIBUTE_FORMAT_UINT32: return VK_FORMAT_R32_UINT;
        case CO_ATTRIBUTE_FORMAT_UINT16: return VK_FORMAT_R16_UINT;
        default: assert(false);          return VK_FORMAT_UNDEFINED;
    }
}

VkPolygonMode
convert(CoPolygonMode mode)
{
    switch (mode)
    {
        case CO_POLYGON_MODE_WIREFRAME: return VK_POLYGON_MODE_LINE;
        case CO_POLYGON_MODE_SOLID:     return VK_POLYGON_MODE_FILL;
        case CO_POLYGON_MODE_POINTS:    return VK_POLYGON_MODE_POINT;
        default: assert(false);         return VK_POLYGON_MODE_FILL;
    }
}


VkCullModeFlags
convert(CoCullMode mode)
{
    switch (mode)
    {
        case CO_CULL_MODE_NONE:           return VK_CULL_MODE_NONE;
        case CO_CULL_MODE_FRONT_AND_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
        case CO_CULL_MODE_BACK:           return VK_CULL_MODE_BACK_BIT;
        case CO_CULL_MODE_FRONT:          return VK_CULL_MODE_FRONT_BIT;
        default: assert(false);           return VK_CULL_MODE_NONE;
    }
}


VkFrontFace
convert(CoFrontFaceOrientation orientation)
{
    switch (orientation)
    {
        case CO_FRONT_FACE_ORIENTATION_CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case CO_FRONT_FACE_ORIENTATION_CW:  return VK_FRONT_FACE_CLOCKWISE;
        default: assert(false);             return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}


VkPrimitiveTopology
convert(CoTopology topology)
{
    switch (topology)
    {
        case CO_TOPOLOGY_POINT_LIST:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case CO_TOPOLOGY_LINE_LIST:     return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case CO_TOPOLOGY_TRIANGLE_LIST: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    std::unreachable();
}


template<typename T>
VkDescriptorType
toVkDescriptorType()
{
    static_assert(false);
}

VkDescriptorType
convert(CoDescriptorType type)
{
    switch (type)
    {
        case CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case CO_DESCRIPTOR_TYPE_STORAGE_BUFFER:         return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case CO_DESCRIPTOR_TYPE_IMAGE:                  return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case CO_DESCRIPTOR_TYPE_SAMPLER:                return VK_DESCRIPTOR_TYPE_SAMPLER;
        case CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    std::unreachable();
}

} // namespace


GraphicsPipelineImpl::~GraphicsPipelineImpl()
{
    if (mPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(context().getVkDevice(), mPipeline, nullptr);
    }

    if (mPipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(context().getVkDevice(), mPipelineLayout, nullptr);
    }
}


std::optional<Coral::GraphicsPipeline::CreateError>
GraphicsPipelineImpl::init(const Coral::GraphicsPipeline::CreateConfig& config)
{
    //-------------------------------------------------------------
    // Shader Stage State
    //-------------------------------------------------------------

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    Coral::Vulkan::ShaderModuleImpl* vertexShader{ nullptr };
    for (const auto& shaderModule : config.shaderModules)
    {
        auto shader = static_cast<Coral::Vulkan::ShaderModuleImpl*>(shaderModule.get());

        if (shader->shaderStage() == CO_SHADER_STAGE_VERTEX)
        {
            vertexShader = shader;
        }

        VkPipelineShaderStageCreateInfo shaderStage
        { 
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = ::convert(shaderModule->shaderStage()),
            .module = shader->getVkShaderModule(),
            .pName  = shader->entryPoint().c_str(),
        };
        shaderStages.push_back(shaderStage);
    }

    if (!vertexShader)
    {
        return Coral::GraphicsPipeline::CreateError::INTERNAL_ERROR;
    }
    
    //-------------------------------------------------------------
    // Rasterization State
    //-------------------------------------------------------------

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo
    { 
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = ::convert(config.polygonMode),
        .cullMode                = ::convert(config.faceCullingMode.cullMode),
        .frontFace               = ::convert(config.faceCullingMode.orientation),
        // TODO: Enable depth bias
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0.f,
        .depthBiasClamp          = 0.f,
        .depthBiasSlopeFactor    = 0.f,
        .lineWidth               = 1.f,
    };

    //-------------------------------------------------------------
    // Depth Stencil State
    //-------------------------------------------------------------

    // TODO: Enable depth test
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo
    { 
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable       = VK_TRUE,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
    };

    //-------------------------------------------------------------
    // Viewport State
    //-------------------------------------------------------------

    VkPipelineViewportStateCreateInfo viewportCreateInfo
    { 
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports    = nullptr,
        .scissorCount  = 1,
        .pScissors     = nullptr,
    };

    //-------------------------------------------------------------
    // Multisample State
    //-------------------------------------------------------------

    // TODO: Implement Multi sampling
    VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo
    { 
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = VK_FALSE,
        .minSampleShading      = 1.f,
        .pSampleMask           = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable      = VK_FALSE,
    };

    //-------------------------------------------------------------
    // Color Blend State
    //-------------------------------------------------------------

    // TODO: Implement Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable         = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments    = &colorBlendAttachment;
    colorBlendCreateInfo.logicOpEnable   = VK_FALSE;
    colorBlendCreateInfo.logicOp         = VK_LOGIC_OP_COPY;

    //-------------------------------------------------------------
    // Vertex Input State
    //-------------------------------------------------------------

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    auto& vertexShaderLayout = vertexShader->layout();
    for (const auto& info : std::span(vertexShaderLayout.inputAttributeLayout.pAttributeBindingInfos, 
                                      vertexShaderLayout.inputAttributeLayout.attributeBindingInfoCount))
    {
        auto& bindingDescription     = bindingDescriptions.emplace_back();
        bindingDescription.binding   = info.location;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride    = 0;

        auto& attributeDescription    = attributeDescriptions.emplace_back();
        attributeDescription.binding  = info.location;
        attributeDescription.location = info.location;
        attributeDescription.format   = ::convert(info.format);
        attributeDescription.offset   = 0;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo
    {   
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = static_cast<uint32_t>(bindingDescriptions.size()),
        .pVertexBindingDescriptions      = bindingDescriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions    = attributeDescriptions.data(),
    };

    //-------------------------------------------------------------
    // Dynamic State
    //-------------------------------------------------------------

    // The coral API allows to set the viewport and line width via command buffer
    std::vector dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
        /*VK_DYNAMIC_STATE_LINE_WIDTH*/
    };

    if (!bindingDescriptions.empty())
    {
        dynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    }

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo
    { 
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates    = dynamicStates.data(),
    };

    //-------------------------------------------------------------
    // Input Assembly State
    //-------------------------------------------------------------

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo
    { 
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = ::convert(config.topology),
        .primitiveRestartEnable = VK_FALSE,
    };

    //-------------------------------------------------------------
    // Rendering State
    //-------------------------------------------------------------

    std::vector<VkFormat> colorAttachments;
    for (auto format : config.framebufferLayout.colorAttachments)
    {
        colorAttachments.push_back(Coral::Vulkan::convert(format.format));
    };

    VkFormat depthStencilFormat{ VK_FORMAT_UNDEFINED };
    if (config.framebufferLayout.depthStencilAttachment)
    {
        depthStencilFormat = Coral::Vulkan::convert(config.framebufferLayout.depthStencilAttachment->format);
    }

    VkPipelineRenderingCreateInfo renderingCreateInfo
    { 
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = static_cast<uint32_t>(colorAttachments.size()),
        .pColorAttachmentFormats = colorAttachments.data(),
        .depthAttachmentFormat   = depthStencilFormat,
        .stencilAttachmentFormat = depthStencilFormat,
    };

    //-------------------------------------------------------------
    // Descriptor Set Layout
    //-------------------------------------------------------------

    struct DescriptorSetData
    {
        uint32_t set{ 0 };
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    std::unordered_map<uint32_t, DescriptorSetData> sets;

    for (auto shader : config.shaderModules)
    {
        auto stage  = ::convert(shader->shaderStage());
        auto layout = shader->layout();
        for (const auto& set : std::span(layout.pDescriptorSetLayouts, 
                                         layout.descriptorSetLayoutCount))
        {
            auto& resolved = sets[set.set];
            resolved.set   = set.set;

            for (const auto& info : std::span(set.pDescriptorInfos, set.descriptorInfoCount))
            { 
                if (std::ranges::contains(resolved.bindings, info.binding, &VkDescriptorSetLayoutBinding::binding))
                {
                    continue;
                }

                auto& binding = resolved.bindings.emplace_back();
                binding.binding         = info.binding;
                binding.descriptorCount = 1;
                binding.stageFlags      = VK_SHADER_STAGE_ALL;
                binding.descriptorType  = ::convert(info.type);
            }
        }
    }

    std::vector<VkDescriptorSetLayout> layouts;
    for (const auto& [_, set] : sets)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        descriptorSetLayoutCreateInfo.pBindings    = set.bindings.data();
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(set.bindings.size());
        //descriptorSetLayoutCreateInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

        auto& vkSet = layouts.emplace_back(VK_NULL_HANDLE);
        if (vkCreateDescriptorSetLayout(context().getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &vkSet) != VK_SUCCESS)
        {
            return GraphicsPipeline::CreateError::INTERNAL_ERROR;
        }
    }

    Finally destroyLayouts([&]
    {
        for (auto layout : layouts)
        {
            if (layout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(context().getVkDevice(), layout, nullptr);
            }
        }
    });

    //-------------------------------------------------------------
    // Pipeline Layout 
    //-------------------------------------------------------------

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
    {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO ,
        .setLayoutCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts    = layouts.data(),
    };

    if (vkCreatePipelineLayout(context().getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        return GraphicsPipeline::CreateError::INTERNAL_ERROR;
    }

    //-------------------------------------------------------------
    // Create Graphics Pipeline
    //-------------------------------------------------------------

    VkGraphicsPipelineCreateInfo createInfo
    { 
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingCreateInfo,
        .stageCount          = static_cast<uint32_t>(shaderStages.size()),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputCreateInfo,
        .pInputAssemblyState = &inputAssemblyCreateInfo,
        .pViewportState      = &viewportCreateInfo,
        .pRasterizationState = &rasterizationCreateInfo,
        .pMultisampleState   = &multiSamplingCreateInfo,
        .pDepthStencilState  = &depthStencilCreateInfo,
        .pColorBlendState    = &colorBlendCreateInfo,
        .pDynamicState       = &dynamicStateCreateInfo,
        .layout              = mPipelineLayout,
    };

    if (vkCreateGraphicsPipelines(context().getVkDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        return GraphicsPipeline::CreateError::INTERNAL_ERROR;
    }

    return {};
}


VkPipeline
GraphicsPipelineImpl::getVkPipeline()
{
    return mPipeline;
}


VkPipelineLayout
GraphicsPipelineImpl::getVkPipelineLayout()
{
    return mPipelineLayout;
}
