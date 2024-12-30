module;

#include <vulkan/vulkan.h>

#include <cassert>
#include <string>
#include <span>
#include <vector>
#include <array>
#include <unordered_map>

module Coral.Vulkan.PipelineLayout;

import Coral.Vulkan.ShaderModule;
import Coral.Vulkan.Format;

using namespace Coral::Vulkan;


PipelineLayout::~PipelineLayout()
{
	if (mContext)
	{
		if (mPipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(mContext->getVkDevice(), mPipelineLayout, nullptr);
		}

		for (auto layout : mDescriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(mContext->getVkDevice(), layout, nullptr);
		}
	}
}


VkPipelineLayout
PipelineLayout::getVkPipelineLayout()
{
	return mPipelineLayout;
}


bool
PipelineLayout::init(Coral::Vulkan::Context* context, const Coral::PipelineLayoutConfig& config)
{
	if (!context)
	{
		return false;
	}

	mContext = context;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	Coral::Vulkan::ShaderModule* vertexShader{ nullptr };
	for (const auto& shaderModule : config.shaderModules)
	{
		auto shader = static_cast<Coral::Vulkan::ShaderModule*>(shaderModule);

		if (shader->shaderStage() == Coral::ShaderStage::VERTEX)
		{
			vertexShader = shader;
		}
		VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStage.module	= shader->getVkShaderModule();
		shaderStage.stage	= ::convert(shaderModule->shaderStage());
		shaderStage.pName	= shader->entryPoint().data();
		shaderStages.push_back(shaderStage);
	}
	
	VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationCreateInfo.polygonMode				= ::convert(config.polygonMode);
	rasterizationCreateInfo.cullMode				= ::convert(config.faceCullingMode.cullMode);
	rasterizationCreateInfo.frontFace				= ::convert(config.faceCullingMode.orientation);
	rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationCreateInfo.lineWidth				= 1.f;
	// TODO: Enable depth bias
	rasterizationCreateInfo.depthBiasEnable			= VK_FALSE;
	rasterizationCreateInfo.depthBiasConstantFactor = 0.f;
	rasterizationCreateInfo.depthBiasClamp			= 0.f;
	rasterizationCreateInfo.depthBiasSlopeFactor	= 0.f;

	// TODO: Enable depth test
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilCreateInfo.stencilTestEnable		= VK_FALSE;
	depthStencilCreateInfo.depthTestEnable			= VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable			= VK_TRUE;
	depthStencilCreateInfo.depthCompareOp			= VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable	= VK_FALSE;

	VkViewport viewport;
	viewport.x			= config.viewport.x;
	viewport.y			= config.viewport.y;
	viewport.width		= config.viewport.width;
	viewport.height		= config.viewport.height;
	viewport.minDepth	= 0;
	viewport.maxDepth	= 1;

	VkRect2D rect;
	rect.extent.width	= viewport.width;
	rect.extent.height	= viewport.height;
	rect.offset.x		= 0;
	rect.offset.y		= 0;

	VkPipelineViewportStateCreateInfo viewPortCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewPortCreateInfo.pViewports		= &viewport;
	viewPortCreateInfo.viewportCount	= 1;
	viewPortCreateInfo.pScissors		= &rect;
	viewPortCreateInfo.scissorCount		= 1;

	// TODO: Implement Multi sampling
	VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multiSamplingCreateInfo.sampleShadingEnable		= VK_FALSE;
	multiSamplingCreateInfo.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multiSamplingCreateInfo.minSampleShading		= 1.f;
	multiSamplingCreateInfo.pSampleMask				= nullptr;
	multiSamplingCreateInfo.alphaToCoverageEnable	= VK_FALSE;
	multiSamplingCreateInfo.alphaToOneEnable		= VK_FALSE;

	// TODO: Implement Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{  };
	colorBlendAttachment.blendEnable			= VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendCreateInfo.attachmentCount	= 1;
	colorBlendCreateInfo.pAttachments		= &colorBlendAttachment;
	colorBlendCreateInfo.logicOpEnable		= VK_FALSE;
	colorBlendCreateInfo.logicOp			= VK_LOGIC_OP_COPY;

	// The coral API allows to set the viewport and line width via command buffer
	std::array<VkDynamicState, 0> dynamicStates = {
		/*VK_DYNAMIC_STATE_VIEWPORT*/
		/*VK_DYNAMIC_STATE_LINE_WIDTH*/
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCreateInfo.pDynamicStates		= dynamicStates.data();
	dynamicStateCreateInfo.dynamicStateCount	= static_cast<uint32_t>(dynamicStates.size());

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	uint32_t i{ 0 };
	for (const auto& description : config.vertexInputAttributeDescriptions)
	{
		auto& bindingDescription		= bindingDescriptions.emplace_back();
		bindingDescription.binding		= i;
		bindingDescription.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescription.stride		= description.stride > 0 ? description.stride : sizeInBytes(description.format);
		
		auto& attributeDescription		= attributeDescriptions.emplace_back();
		attributeDescription.binding	= bindingDescription.binding;
		attributeDescription.format		= ::convert(description.format);
		attributeDescription.offset		= description.offset;
		i++;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputCreateInfo.vertexBindingDescriptionCount		= static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputCreateInfo.pVertexBindingDescriptions		= bindingDescriptions.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions		= attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyCreateInfo.primitiveRestartEnable	= VK_FALSE;
	inputAssemblyCreateInfo.topology				= ::convert(config.topology);

	
	std::unordered_map<uint32_t, std::vector<BindingDescription>> descriptorSets;

	for (auto binding : vertexShader->bindingDescriptions())
	{
		descriptorSets[binding.set].push_back(binding);
	}

	for (auto [set, bindingDescriptions] : descriptorSets)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto bindingDescription : bindingDescriptions)
		{
			auto& binding			= bindings.emplace_back();
			binding.binding			= bindingDescription.binding;
			binding.descriptorCount = 1;
			binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			binding.stageFlags		= VK_SHADER_STAGE_ALL_GRAPHICS;
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descriptorSetLayoutCreateInfo.pBindings		= bindings.data();
		descriptorSetLayoutCreateInfo.bindingCount	= static_cast<uint32_t>(bindings.size());

		VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
		if (vkCreateDescriptorSetLayout(mContext->getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
		{
			return false;
		}

		mDescriptorSetLayouts.push_back(layout);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(mDescriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts	= mDescriptorSetLayouts.data();

	if (vkCreatePipelineLayout(mContext->getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
	{
		return false;
	}

	std::vector<VkFormat> colorAttachments;

	for (auto attachment: config.colorAttachmentDescription)
	{
		colorAttachments.push_back(Coral::Vulkan::convert(attachment.format));
	};

	VkFormat depthStencilFormat{ VK_FORMAT_UNDEFINED };
	if (config.depthStencilAttachmentDescription)
	{
		depthStencilFormat = Coral::Vulkan::convert(config.depthStencilAttachmentDescription->format);
	}
	VkPipelineRenderingCreateInfo renderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	renderingCreateInfo.colorAttachmentCount	= static_cast<uint32_t>(colorAttachments.size());
	renderingCreateInfo.pColorAttachmentFormats = colorAttachments.data();
	renderingCreateInfo.depthAttachmentFormat	= depthStencilFormat;
	renderingCreateInfo.stencilAttachmentFormat = depthStencilFormat;

	VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	createInfo.stageCount			= static_cast<uint32_t>(shaderStages.size());
	createInfo.pStages				= shaderStages.data();
	createInfo.pDynamicState		= &dynamicStateCreateInfo;
	createInfo.pVertexInputState	= &vertexInputCreateInfo;

	createInfo.pRasterizationState	= &rasterizationCreateInfo;
	createInfo.pDepthStencilState	= &depthStencilCreateInfo;
	createInfo.pViewportState		= &viewPortCreateInfo;
	createInfo.pMultisampleState	= &multiSamplingCreateInfo;
	createInfo.pColorBlendState		= &colorBlendCreateInfo;
	createInfo.pInputAssemblyState  = &inputAssemblyCreateInfo;
	createInfo.layout				= mPipelineLayout;
	createInfo.pNext				= &renderingCreateInfo;

	if (vkCreateGraphicsPipelines(mContext->getVkDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

