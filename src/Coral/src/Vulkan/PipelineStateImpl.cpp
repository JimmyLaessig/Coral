
#include "PipelineStateImpl.hpp"

#include "ShaderModuleImpl.hpp"
#include "../Visitor.hpp"
#include "VulkanFormat.hpp"

#include <array>
#include <cassert>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>


using namespace Coral::Vulkan;

namespace
{

VkShaderStageFlagBits
convert(Coral::ShaderStage shaderStage)
{
	switch (shaderStage)
	{
		case Coral::ShaderStage::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case Coral::ShaderStage::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:
			assert(false);
			return VK_SHADER_STAGE_ALL_GRAPHICS;
	}
}


VkFormat
convert(Coral::AttributeFormat format)
{
	switch (format)
	{
		case Coral::AttributeFormat::VEC4F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Coral::AttributeFormat::VEC3F:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Coral::AttributeFormat::VEC2F:
			return VK_FORMAT_R32G32_SFLOAT;
		case Coral::AttributeFormat::FLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case Coral::AttributeFormat::INT32:
			return VK_FORMAT_R32_SINT;
		case Coral::AttributeFormat::INT16:
			return VK_FORMAT_R16_SINT;
		case Coral::AttributeFormat::UINT32:
			return VK_FORMAT_R32_UINT;
		case Coral::AttributeFormat::UINT16:
			return VK_FORMAT_R16_UINT;
		default:
			assert(false);
			return VK_FORMAT_UNDEFINED;
	}
}

VkPolygonMode
convert(Coral::PolygonMode mode)
{
	switch (mode)
	{
		case Coral::PolygonMode::WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case Coral::PolygonMode::SOLID:
			return VK_POLYGON_MODE_FILL;
		case Coral::PolygonMode::POINTS:
			return VK_POLYGON_MODE_POINT;
		default:
			assert(false);
			return VK_POLYGON_MODE_FILL;
	}
}


VkCullModeFlags
convert(Coral::CullMode mode)
{
	switch (mode)
	{
		case Coral::CullMode::NONE:
			return VK_CULL_MODE_NONE;
		case Coral::CullMode::FRONT_AND_BACK:
			return VK_CULL_MODE_FRONT_AND_BACK;
		case Coral::CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		case Coral::CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		default:
			assert(false);
			return VK_CULL_MODE_NONE;
	}
}


VkFrontFace
convert(Coral::FrontFaceOrientation orientation)
{
	switch (orientation)
	{
		case Coral::FrontFaceOrientation::CCW:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		case Coral::FrontFaceOrientation::CW:
			return VK_FRONT_FACE_CLOCKWISE;
		default:
			assert(false);
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}


VkPrimitiveTopology
convert(Coral::Topology topology)
{
	switch (topology)
	{
		case Coral::Topology::POINT_LIST:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case Coral::Topology::LINE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case Coral::Topology::TRIANGLE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		default:
			assert(false);
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

} // namespace


PipelineStateImpl::~PipelineStateImpl()
{
	if (mContext)
	{
		if (mPipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(mContext->getVkDevice(), mPipeline, nullptr);
		}

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


VkPipeline
PipelineStateImpl::getVkPipeline()
{
	return mPipeline;
}


std::span<VkDescriptorSetLayout>
PipelineStateImpl::getVkDescriptorSetLayouts()
{
	return mDescriptorSetLayouts;
}


VkPipelineLayout
PipelineStateImpl::getVkPipelineLayout()
{
	return mPipelineLayout;
}


std::optional<Coral::PipelineStateCreationError>
PipelineStateImpl::init(Coral::Vulkan::ContextImpl& context, const Coral::PipelineStateCreateConfig& config)
{
	mContext = &context;

	//-------------------------------------------------------------
	// Shader Stage State
	//-------------------------------------------------------------

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	Coral::Vulkan::ShaderModuleImpl* vertexShader{ nullptr };
	for (const auto& shaderModule : config.shaderModules)
	{
		auto shader = static_cast<Coral::Vulkan::ShaderModuleImpl*>(shaderModule);

		if (shader->shaderStage() == Coral::ShaderStage::VERTEX)
		{
			vertexShader = shader;
		}

		VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStage.module	= shader->getVkShaderModule();
		shaderStage.stage	= ::convert(shaderModule->shaderStage());
		shaderStage.pName	= shader->entryPoint().c_str();
		shaderStages.push_back(shaderStage);
	}
	
	//-------------------------------------------------------------
	// Rasterization State
	//-------------------------------------------------------------

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

	//-------------------------------------------------------------
	// Depth Stencil State
	//-------------------------------------------------------------

	// TODO: Enable depth test
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilCreateInfo.stencilTestEnable		= VK_FALSE;
	depthStencilCreateInfo.depthTestEnable			= VK_TRUE;
	depthStencilCreateInfo.depthWriteEnable			= VK_TRUE;
	depthStencilCreateInfo.depthCompareOp			= VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable	= VK_FALSE;

	//-------------------------------------------------------------
	// Viewport State
	//-------------------------------------------------------------

	VkPipelineViewportStateCreateInfo viewportCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportCreateInfo.pViewports		= nullptr;
	viewportCreateInfo.viewportCount	= 1;
	viewportCreateInfo.pScissors		= nullptr;
	viewportCreateInfo.scissorCount		= 1;

	//-------------------------------------------------------------
	// Multisample State
	//-------------------------------------------------------------

	// TODO: Implement Multi sampling
	VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multiSamplingCreateInfo.sampleShadingEnable		= VK_FALSE;
	multiSamplingCreateInfo.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multiSamplingCreateInfo.minSampleShading		= 1.f;
	multiSamplingCreateInfo.pSampleMask				= nullptr;
	multiSamplingCreateInfo.alphaToCoverageEnable	= VK_FALSE;
	multiSamplingCreateInfo.alphaToOneEnable		= VK_FALSE;

	//-------------------------------------------------------------
	// Color Blend State
	//-------------------------------------------------------------

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

	//-------------------------------------------------------------
	// Dynamic State
	//-------------------------------------------------------------

	// The coral API allows to set the viewport and line width via command buffer
	std::array dynamicStates = {

		VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
		/*VK_DYNAMIC_STATE_LINE_WIDTH*/
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCreateInfo.pDynamicStates		= dynamicStates.data();
	dynamicStateCreateInfo.dynamicStateCount	= static_cast<uint32_t>(dynamicStates.size());

	//-------------------------------------------------------------
	// Vertex Input State
	//-------------------------------------------------------------

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	for (const auto& description : vertexShader->inputAttributeBindingDefinitions())
	{
		auto& bindingDescription		= bindingDescriptions.emplace_back();
		bindingDescription.binding		= description.binding;
		bindingDescription.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescription.stride		= 0;

		auto& attributeDescription		= attributeDescriptions.emplace_back();
		attributeDescription.binding	= description.binding;
		attributeDescription.location	= description.location;
		attributeDescription.format		= ::convert(description.format);
		attributeDescription.offset		= 0;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputCreateInfo.vertexBindingDescriptionCount		= static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputCreateInfo.pVertexBindingDescriptions		= bindingDescriptions.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions		= attributeDescriptions.data();

	//-------------------------------------------------------------
	// Input Assembly State
	//-------------------------------------------------------------

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyCreateInfo.primitiveRestartEnable	= VK_FALSE;
	inputAssemblyCreateInfo.topology				= ::convert(config.topology);

	//-------------------------------------------------------------
	// Rendering State
	//-------------------------------------------------------------

	std::vector<VkFormat> colorAttachments;
	for (auto format : config.framebufferSignature.colorAttachmentFormats)
	{
		colorAttachments.push_back(Coral::Vulkan::convert(format));
	};

	VkFormat depthStencilFormat{ VK_FORMAT_UNDEFINED };
	if (config.framebufferSignature.depthStencilAttachmentFormat)
	{
		depthStencilFormat = Coral::Vulkan::convert(*config.framebufferSignature.depthStencilAttachmentFormat);
	}
	VkPipelineRenderingCreateInfo renderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	renderingCreateInfo.colorAttachmentCount	= static_cast<uint32_t>(colorAttachments.size());
	renderingCreateInfo.pColorAttachmentFormats = colorAttachments.data();
	renderingCreateInfo.depthAttachmentFormat	= depthStencilFormat;
	renderingCreateInfo.stencilAttachmentFormat = depthStencilFormat;

	//-------------------------------------------------------------
	// Descriptor Set Layouts
	//-------------------------------------------------------------

	std::unordered_map<uint32_t, std::unordered_map<uint32_t, Coral::DescriptorBindingDefinition>> descriptorSets;

	for (auto shader : config.shaderModules)
	{
		for (const auto& bufferBindingDescription : shader->descriptorBindingDefinitions())
		{
			descriptorSets[bufferBindingDescription.set][bufferBindingDescription.binding] = bufferBindingDescription;
		}
	}

	for (const auto& [set, bindingDescriptions] : descriptorSets)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto [_, bindingDescription] : bindingDescriptions)
		{
			auto& binding			= bindings.emplace_back();
			binding.binding			= bindingDescription.binding;
			binding.descriptorCount = 1;
			binding.stageFlags		= VK_SHADER_STAGE_ALL_GRAPHICS;
			
			binding.descriptorType = std::visit(Coral::Visitor{
				[](const SamplerDefinition&) { return VK_DESCRIPTOR_TYPE_SAMPLER; },
				[](const TextureDefinition&) { return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; },
				[](const UniformBlockDefinition&) { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }, 
				[](const CombinedTextureSamplerDefinition&) { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; },
				}, bindingDescription.definition);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		descriptorSetLayoutCreateInfo.pBindings		= bindings.data();
		descriptorSetLayoutCreateInfo.bindingCount	= static_cast<uint32_t>(bindings.size());

		VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
		if (vkCreateDescriptorSetLayout(mContext->getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
		{
			return PipelineStateCreationError::INTERNAL_ERROR;
		}

		mDescriptorSetLayouts.push_back(layout);
	}

	//-------------------------------------------------------------
	// Pipeline Layout 
	//-------------------------------------------------------------

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(mDescriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts	= mDescriptorSetLayouts.data();

	if (vkCreatePipelineLayout(mContext->getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
	{
		return PipelineStateCreationError::INTERNAL_ERROR;
	}

	//-------------------------------------------------------------
	// Create Graphics Pipeline
	//-------------------------------------------------------------

	VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	createInfo.stageCount			= static_cast<uint32_t>(shaderStages.size());
	createInfo.pStages				= shaderStages.data();
	createInfo.pDynamicState		= &dynamicStateCreateInfo;
	createInfo.pVertexInputState	= &vertexInputCreateInfo;

	createInfo.pRasterizationState	= &rasterizationCreateInfo;
	createInfo.pDepthStencilState	= &depthStencilCreateInfo;
	createInfo.pViewportState		= &viewportCreateInfo;
	createInfo.pMultisampleState	= &multiSamplingCreateInfo;
	createInfo.pColorBlendState		= &colorBlendCreateInfo;
	createInfo.pInputAssemblyState  = &inputAssemblyCreateInfo;
	createInfo.layout				= mPipelineLayout;
	createInfo.pNext				= &renderingCreateInfo;

	if (vkCreateGraphicsPipelines(mContext->getVkDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline) != VK_SUCCESS)
	{
		return PipelineStateCreationError::INTERNAL_ERROR;
	}

	return {};
}

