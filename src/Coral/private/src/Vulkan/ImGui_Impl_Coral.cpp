#include <Coral/ImGui_Impl_Coral.hpp>

#include <backends/imgui_impl_vulkan.h>

#include <Coral/Vulkan/Vulkan.hpp>
#include <Coral/Vulkan/CommandBufferImpl.hpp>
#include <Coral/Vulkan/CommandQueueImpl.hpp>
#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>


bool
ImGui_ImplCoral_Init(ImGui_ImplCoral_InitInfo* initInfo)
{
	auto context = static_cast<Coral::Vulkan::ContextImpl*>(initInfo->context);
	auto queue   = static_cast<Coral::Vulkan::CommandQueueImpl*>(context->getGraphicsQueue());

	std::vector<VkFormat> colorAttachments;
	for (auto format : initInfo->framebufferSignature.colorAttachmentFormats)
	{
		colorAttachments.push_back(Coral::Vulkan::convert(format));
	}
	
	VkPipelineRenderingCreateInfo renderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	renderingCreateInfo.colorAttachmentCount    = static_cast<uint32_t>(colorAttachments.size());
	renderingCreateInfo.pColorAttachmentFormats = colorAttachments.data();
	renderingCreateInfo.depthAttachmentFormat   = VK_FORMAT_UNDEFINED;
	renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	
	if (auto format = initInfo->framebufferSignature.depthStencilAttachmentFormat)
	{
		auto vkFormat = Coral::Vulkan::convert(*format);
		renderingCreateInfo.depthAttachmentFormat = vkFormat;

		if (Coral::Vulkan::isStencilFormat(*format))
		{
			renderingCreateInfo.stencilAttachmentFormat = vkFormat;
		}
	}

	ImGui_ImplVulkan_InitInfo vkInitInfo{};
	vkInitInfo.ApiVersion                  = VK_API_VERSION_1_3;
	vkInitInfo.Instance                    = context->getVkInstance();
	vkInitInfo.PhysicalDevice              = context->getVkPhysicalDevice();
	vkInitInfo.Device                      = context->getVkDevice();
	vkInitInfo.QueueFamily                 = context->getQueueFamilyIndex();
	vkInitInfo.Queue                       = queue->getVkQueue();
	vkInitInfo.DescriptorPool              = VK_NULL_HANDLE;
	vkInitInfo.RenderPass                  = VK_NULL_HANDLE;
	vkInitInfo.MinImageCount               = initInfo->swapchainImageCount;
	vkInitInfo.ImageCount                  = initInfo->swapchainImageCount;
	vkInitInfo.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
	vkInitInfo.PipelineCache               = VK_NULL_HANDLE;
	vkInitInfo.Subpass                     = 0;
	vkInitInfo.DescriptorPoolSize          = 25;
	vkInitInfo.UseDynamicRendering         = true;
	vkInitInfo.PipelineRenderingCreateInfo = renderingCreateInfo;
	vkInitInfo.Allocator                   = VK_NULL_HANDLE;
	vkInitInfo.CheckVkResultFn             = nullptr;
	vkInitInfo.MinAllocationSize           = 1024 * 1024;

	return ImGui_ImplVulkan_Init(&vkInitInfo);
}


bool
ImGui_ImplCoral_CreateFontsTexture()
{
	return ImGui_ImplVulkan_CreateFontsTexture();
}


void
ImGui_ImplCoral_DestroyFontsTexture()
{
	ImGui_ImplVulkan_DestroyFontsTexture();
}


void
ImGui_ImplCoral_RenderDrawData(ImDrawData* drawData, Coral::CommandBuffer* commandBuffer)
{
	auto commandBufferImpl = static_cast<Coral::Vulkan::CommandBufferImpl*> (commandBuffer);

	ImGui_ImplVulkan_RenderDrawData(drawData, commandBufferImpl->getVkCommandBuffer(), VK_NULL_HANDLE);
}


void
ImGui_ImplCoral_NewFrame()
{
	ImGui_ImplVulkan_NewFrame();
}


void
ImGui_ImplCoral_Shutdown()
{
	ImGui_ImplVulkan_Shutdown();
}
