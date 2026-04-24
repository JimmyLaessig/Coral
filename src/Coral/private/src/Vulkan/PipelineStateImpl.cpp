
#include <Coral/Vulkan/PipelineStateImpl.hpp>

#include <Coral/Visitor.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>
#include <Coral/Vulkan/ShaderModuleImpl.hpp>

#include <array>
#include <cassert>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <unordered_set>

using namespace Coral::Vulkan;

namespace
{

VkShaderStageFlagBits
convert(CoShaderStage shaderStage)
{
	switch (shaderStage)
	{
		case CO_SHADER_STAGE_VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case CO_SHADER_STAGE_FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		default:
			assert(false);
			return VK_SHADER_STAGE_ALL_GRAPHICS;
	}
}


VkFormat
convert(CoAttributeFormat format)
{
	switch (format)
	{
		case CO_ATTRIBUTE_FORMAT_VEC4F: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case CO_ATTRIBUTE_FORMAT_VEC3F: return VK_FORMAT_R32G32B32_SFLOAT;
		case CO_ATTRIBUTE_FORMAT_VEC2F: return VK_FORMAT_R32G32_SFLOAT;
		case CO_ATTRIBUTE_FORMAT_FLOAT: return VK_FORMAT_R32_SFLOAT;
		case CO_ATTRIBUTE_FORMAT_INT32: return VK_FORMAT_R32_SINT;
		case CO_ATTRIBUTE_FORMAT_INT16: return VK_FORMAT_R16_SINT;
		case CO_ATTRIBUTE_FORMAT_UINT32: return VK_FORMAT_R32_UINT;
		case CO_ATTRIBUTE_FORMAT_UINT16: return VK_FORMAT_R16_UINT;
		default:
			assert(false);
			return VK_FORMAT_UNDEFINED;
	}
}

VkPolygonMode
convert(CoPolygonMode mode)
{
	switch (mode)
	{
		case CO_POLYGON_MODE_WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case CO_POLYGON_MODE_SOLID:
			return VK_POLYGON_MODE_FILL;
		case CO_POLYGON_MODE_POINTS:
			return VK_POLYGON_MODE_POINT;
		default:
			assert(false);
			return VK_POLYGON_MODE_FILL;
	}
}


VkCullModeFlags
convert(CoCullMode mode)
{
	switch (mode)
	{
		case CO_CULL_MODE_NONE:
			return VK_CULL_MODE_NONE;
		case CO_CULL_MODE_FRONT_AND_BACK:
			return VK_CULL_MODE_FRONT_AND_BACK;
		case CO_CULL_MODE_BACK:
			return VK_CULL_MODE_BACK_BIT;
		case CO_CULL_MODE_FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		default:
			assert(false);
			return VK_CULL_MODE_NONE;
	}
}


VkFrontFace
convert(CoFrontFaceOrientation orientation)
{
	switch (orientation)
	{
		case CO_FRONT_FACE_ORIENTATION_CCW:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		case CO_FRONT_FACE_ORIENTATION_CW:
			return VK_FRONT_FACE_CLOCKWISE;
		default:
			assert(false);
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}


VkPrimitiveTopology
convert(CoTopology topology)
{
	switch (topology)
	{
		case CO_TOPOLOGY_POINT_LIST:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case CO_TOPOLOGY_LINE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case CO_TOPOLOGY_TRIANGLE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		default:
			assert(false);
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}


template<typename T>
VkDescriptorType
toVkDescriptorType()
{
	static_assert(false);
}

template<>
VkDescriptorType
toVkDescriptorType<Coral::SamplerDefinition>() { return VK_DESCRIPTOR_TYPE_SAMPLER; }

template<>
VkDescriptorType
toVkDescriptorType<Coral::TextureDefinition>() { return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; }

template<>
VkDescriptorType
toVkDescriptorType<Coral::CombinedTextureSamplerDefinition>() { return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; }

template<>
VkDescriptorType
toVkDescriptorType<Coral::UniformBlockDefinition>() { return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; }


} // namespace


PipelineStateImpl::~PipelineStateImpl()
{
	if (mPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(context().getVkDevice(), mPipeline, nullptr);
	}

	if (mPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(context().getVkDevice(), mPipelineLayout, nullptr);
	}

	if (mDescriptorSetLayout != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(context().getVkDevice(), mDescriptorSetLayout, nullptr);
	}
}


std::optional<Coral::PipelineState::CreateError>
PipelineStateImpl::init(const Coral::PipelineState::CreateConfig& config)
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

		VkPipelineShaderStageCreateInfo shaderStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		shaderStage.module	= shader->getVkShaderModule();
		shaderStage.stage	= ::convert(shaderModule->shaderStage());
		shaderStage.pName	= shader->entryPoint().c_str();
		shaderStages.push_back(shaderStage);
	}

	if (!vertexShader)
	{
		return Coral::PipelineState::CreateError::INTERNAL_ERROR;
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

	for (const auto& info : vertexShader->inputAttributeLayout())
	{
		auto& bindingDescription		= bindingDescriptions.emplace_back();
		bindingDescription.binding		= info.location;
		bindingDescription.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescription.stride		= 0;

		auto& attributeDescription		= attributeDescriptions.emplace_back();
		attributeDescription.binding	= info.location;
		attributeDescription.location	= info.location;
		attributeDescription.format		= ::convert(info.format);
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
	for (auto format : config.framebufferLayout.colorAttachments)
	{
		colorAttachments.push_back(Coral::Vulkan::convert(format.format));
	};

	VkFormat depthStencilFormat{ VK_FORMAT_UNDEFINED };
	if (config.framebufferLayout.depthStencilAttachment)
	{
		depthStencilFormat = Coral::Vulkan::convert(config.framebufferLayout.depthStencilAttachment->format);
	}
	VkPipelineRenderingCreateInfo renderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	renderingCreateInfo.colorAttachmentCount	= static_cast<uint32_t>(colorAttachments.size());
	renderingCreateInfo.pColorAttachmentFormats = colorAttachments.data();
	renderingCreateInfo.depthAttachmentFormat	= depthStencilFormat;
	renderingCreateInfo.stencilAttachmentFormat = depthStencilFormat;

	//-------------------------------------------------------------
	// Descriptor Set Layout
	//-------------------------------------------------------------

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	std::unordered_set<uint32_t> visited;
	for (auto shader : config.shaderModules)
	{
		for (const auto& info : shader->descriptorLayout())
		{
			if (!visited.insert(info.binding).second)
			{
				continue;
			}

			auto& binding			= bindings.emplace_back();
			binding.binding			= info.binding;
			binding.descriptorCount = 1;
			binding.stageFlags		= VK_SHADER_STAGE_ALL_GRAPHICS;
			binding.descriptorType = std::visit([](auto a) { return toVkDescriptorType<decltype(a)>(); }, info.definition);
		}
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	descriptorSetLayoutCreateInfo.pBindings		= bindings.data();
	descriptorSetLayoutCreateInfo.bindingCount	= static_cast<uint32_t>(bindings.size());
	descriptorSetLayoutCreateInfo.flags			= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

	VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
	if (vkCreateDescriptorSetLayout(context().getVkDevice(), &descriptorSetLayoutCreateInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
	{
		return PipelineState::CreateError::INTERNAL_ERROR;
	}

	//-------------------------------------------------------------
	// Pipeline Layout 
	//-------------------------------------------------------------

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts	= &mDescriptorSetLayout;

	if (vkCreatePipelineLayout(context().getVkDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
	{
		return PipelineState::CreateError::INTERNAL_ERROR;
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

	if (vkCreateGraphicsPipelines(context().getVkDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mPipeline) != VK_SUCCESS)
	{
		return PipelineState::CreateError::INTERNAL_ERROR;
	}

	return {};
}


VkPipeline
PipelineStateImpl::getVkPipeline()
{
	return mPipeline;
}


std::span<VkDescriptorSetLayout>
PipelineStateImpl::getVkDescriptorSetLayouts()
{
	return { &mDescriptorSetLayout, 1 };
}


VkPipelineLayout
PipelineStateImpl::getVkPipelineLayout()
{
	return mPipelineLayout;
}

