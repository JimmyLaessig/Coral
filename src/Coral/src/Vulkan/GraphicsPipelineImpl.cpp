
#include "GraphicsPipelineImpl.hpp"

#include "ContextImpl.hpp"
#include "ShaderModuleImpl.hpp"
#include "VulkanFormat.hpp"

#include "../Finally.hpp"
#include "../Visitor.hpp"

#include <algorithm>
#include <cassert>
#include <vector>
#include <map>


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
    }

    std::unreachable();
}


VkFormat
convert(CoAttributeFormat format)
{
    switch (format)
    {
        case CO_ATTRIBUTE_FORMAT_R_U16:    return VK_FORMAT_R16_UINT;
        case CO_ATTRIBUTE_FORMAT_R_I16:    return VK_FORMAT_R16_SINT;
        case CO_ATTRIBUTE_FORMAT_R_F16:    return VK_FORMAT_R16_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RG_U16:   return VK_FORMAT_R16G16_UINT;
        case CO_ATTRIBUTE_FORMAT_RG_I16:   return VK_FORMAT_R16G16_SINT;
        case CO_ATTRIBUTE_FORMAT_RG_F16:   return VK_FORMAT_R16G16_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RGB_U16:  return VK_FORMAT_R16G16B16_UINT;
        case CO_ATTRIBUTE_FORMAT_RGB_I16:  return VK_FORMAT_R16G16B16_SINT;
        case CO_ATTRIBUTE_FORMAT_RGB_F16:  return VK_FORMAT_R16G16B16_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RGBA_U16: return VK_FORMAT_R16G16B16A16_UINT;
        case CO_ATTRIBUTE_FORMAT_RGBA_I16: return VK_FORMAT_R16G16B16A16_SINT;
        case CO_ATTRIBUTE_FORMAT_RGBA_F16: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_R_U32:    return VK_FORMAT_R32_UINT;
        case CO_ATTRIBUTE_FORMAT_R_I32:    return VK_FORMAT_R32_SINT;
        case CO_ATTRIBUTE_FORMAT_R_F32:    return VK_FORMAT_R32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RG_U32:   return VK_FORMAT_R32G32_UINT;
        case CO_ATTRIBUTE_FORMAT_RG_I32:   return VK_FORMAT_R32G32_SINT;
        case CO_ATTRIBUTE_FORMAT_RG_F32:   return VK_FORMAT_R32G32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RGB_U32:  return VK_FORMAT_R32G32B32_UINT;
        case CO_ATTRIBUTE_FORMAT_RGB_I32:  return VK_FORMAT_R32G32B32_SINT;
        case CO_ATTRIBUTE_FORMAT_RGB_F32:  return VK_FORMAT_R32G32B32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RGBA_U32: return VK_FORMAT_R32G32B32A32_UINT;
        case CO_ATTRIBUTE_FORMAT_RGBA_I32: return VK_FORMAT_R32G32B32A32_SINT;
        case CO_ATTRIBUTE_FORMAT_RGBA_F32: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_R_U64:    return VK_FORMAT_R64_UINT;
        case CO_ATTRIBUTE_FORMAT_R_I64:    return VK_FORMAT_R64_SINT;
        case CO_ATTRIBUTE_FORMAT_R_F64:    return VK_FORMAT_R64_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RG_U64:   return VK_FORMAT_R64G64_UINT;
        case CO_ATTRIBUTE_FORMAT_RG_I64:   return VK_FORMAT_R64G64_SINT;
        case CO_ATTRIBUTE_FORMAT_RG_F64:   return VK_FORMAT_R64G64_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RGB_U64:  return VK_FORMAT_R64G64B64_UINT;
        case CO_ATTRIBUTE_FORMAT_RGB_I64:  return VK_FORMAT_R64G64B64_SINT;
        case CO_ATTRIBUTE_FORMAT_RGB_F64:  return VK_FORMAT_R64G64B64_SFLOAT;
        case CO_ATTRIBUTE_FORMAT_RGBA_U64: return VK_FORMAT_R64G64B64A64_UINT;
        case CO_ATTRIBUTE_FORMAT_RGBA_I64: return VK_FORMAT_R64G64B64A64_SINT;
        case CO_ATTRIBUTE_FORMAT_RGBA_F64: return VK_FORMAT_R64G64B64A64_SFLOAT;
    }

    std::unreachable();
}


VkPolygonMode
convert(CoPolygonMode mode)
{
    switch (mode)
    {
        case CO_POLYGON_MODE_WIREFRAME: return VK_POLYGON_MODE_LINE;
        case CO_POLYGON_MODE_SOLID:     return VK_POLYGON_MODE_FILL;
        case CO_POLYGON_MODE_POINTS:    return VK_POLYGON_MODE_POINT;
    }

    std::unreachable();
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
    }

    std::unreachable();
}


VkFrontFace
convert(CoFrontFaceOrientation orientation)
{
    switch (orientation)
    {
        case CO_FRONT_FACE_ORIENTATION_CCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case CO_FRONT_FACE_ORIENTATION_CW:  return VK_FRONT_FACE_CLOCKWISE;
    }

    std::unreachable();
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


bool
reflect(const Coral::ReflectDescriptor& descriptor, Coral::BindDescriptorInfo& info)
{
    info.binding = descriptor.binding;

    // TODO: parse array params

    info.type = std::visit(Coral::Visitor{
        [](const Coral::BufferInfo& info)          { return info.bufferType == CO_BUFFER_TYPE_STORAGE ? CO_DESCRIPTOR_TYPE_STORAGE_BUFFER 
                                                                                                      : CO_DESCRIPTOR_TYPE_UNIFORM_BUFFER; },
        [](const Coral::ImageInfo&)                { return CO_DESCRIPTOR_TYPE_IMAGE; },
        [](const Coral::SamplerInfo&)              { return CO_DESCRIPTOR_TYPE_SAMPLER; },
        [](const Coral::CombinedImageSamplerInfo&) { return CO_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }
    }, descriptor.info);

    return true;
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
    std::vector<std::shared_ptr<ShaderModuleImpl>> shaderModules;
    Coral::Vulkan::ShaderModuleImpl* vertexShader{ nullptr };
    for (const auto& shaderModule : config.shaderModules)
    {
        auto shader = std::static_pointer_cast<Coral::Vulkan::ShaderModuleImpl>(shaderModule);
        
        if (shader->shaderStage() == CO_SHADER_STAGE_VERTEX)
        {
            vertexShader = shader.get();
        }

        VkPipelineShaderStageCreateInfo shaderStage
        { 
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = ::convert(shaderModule->shaderStage()),
            .module = shader->getVkShaderModule(),
            .pName  = shader->entryPoint().c_str(),
        };
        shaderStages.push_back(shaderStage);
        shaderModules.push_back(shader);
    }

    if (!vertexShader)
    {
        return Coral::GraphicsPipeline::CreateError::INTERNAL_ERROR;
    }
    
    if (!createDescriptorSetLayouts(shaderModules))
    {
        return Coral::GraphicsPipeline::CreateError::INTERNAL_ERROR;
    }

    if (!createAttributeLayout(*vertexShader))
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
    for (const auto& info : mAttributeLayout.bindings)
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

    std::vector<VkDescriptorSetLayout> layouts;

    for (const auto& layout : mDescriptorSetLayouts)
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkDescriptorBindingFlags> bindingFlags;

        for (const auto& descriptor : layout.bindings)
        {
            auto& binding           = bindings.emplace_back();
            binding.binding         = descriptor.binding;
            binding.descriptorCount = 1;
            binding.stageFlags      = VK_SHADER_STAGE_ALL;
            binding.descriptorType  = ::convert(descriptor.type);

            VkDescriptorBindingFlags flags{ 0 };
            //if (descriptor.type == CO_DESCRIPTOR_TYPE_STORAGE_BUFFER &&
            //    descriptor.buffer.structure.array.dimensionsCount > 0 &&
            //    descriptor.buffer.structure.array.pDimensions[0] == 0)
            //{
            //    flags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
            //}
            bindingFlags.push_back(flags); 
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo
        {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount  = static_cast<uint32_t>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data(),
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
        {
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = &bindingFlagsCreateInfo,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings    = bindings.data(),
        };

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
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
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


bool
GraphicsPipelineImpl::createDescriptorSetLayouts(std::span<const std::shared_ptr<ShaderModuleImpl>> shaderModules)
{
    std::map<uint32_t, DescriptorSetLayout> layouts;

    // Collect all descriptor bindings across all shader modules and join them
    // into one layout for each descriptor set
    for (auto shaderModule : shaderModules)
    {
        const auto& refl = shaderModule->getReflectionInfo();

        for (const auto& descriptorSet : refl.descriptorSets)
        {
            auto& layout = layouts[descriptorSet.set];

            for (const auto& descriptor : descriptorSet.descriptors)
            {
                // The binding has already been parsed
                if (layout.bindings.end() != std::find_if(layout.bindings.begin(), layout.bindings.end(), [&](const auto& info)
                    {
                        return info.binding == descriptor.binding;
                    }))
                {
                    continue;
                }

                auto& target = layout.bindings.emplace_back();

                if (!::reflect(descriptor, target))
                {
                    return false;
                }
            }
        }
    }

    for (auto& [_, set] : layouts)
    {
        mDescriptorSetLayouts.push_back(std::move(set));
    }

    return true;
}


bool
GraphicsPipelineImpl::createAttributeLayout(const ShaderModuleImpl& vertexShader)
{
    const auto& refl = vertexShader.getReflectionInfo();
    for (const auto& attribute : refl.inputAttributes)
    {
        auto& info    = mAttributeLayout.bindings.emplace_back();
        info.location = attribute.location;
        info.format   = attribute.format;
    }
    return true;
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


const std::vector<Coral::DescriptorSetLayout>&
GraphicsPipelineImpl::getDescriptorSetLayouts() const
{
    return mDescriptorSetLayouts;
}


const Coral::AttributeLayout&
GraphicsPipelineImpl::getAttributeLayout() const
{
    return mAttributeLayout;
}
