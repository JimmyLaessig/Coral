
#include <Coral/Vulkan/Vulkan.hpp>
#include <Coral/ImGui_Impl_Coral.hpp>

#include <Coral/Vulkan/ContextImpl.hpp>
#include <Coral/Vulkan/SurfaceImpl.hpp>
#include <Coral/Vulkan/CommandBufferImpl.hpp>
#include <Coral/Vulkan/VulkanFormat.hpp>
#include <backends/imgui_impl_vulkan.h>

bool
ImGui_ImplCoral_Init(ImGui_ImplCoral_InitInfo* initInfo)
{
	auto context = static_cast<Coral::Vulkan::ContextImpl*>(initInfo->context);
	auto surface = static_cast<Coral::Vulkan::SurfaceImpl*>(initInfo->surface);
	auto queue   = static_cast<Coral::Vulkan::CommandQueueImpl*>(context->getGraphicsQueue());

	const auto& surfaceConfig  = surface->config();
	auto colorAttachmentFormat = Coral::Vulkan::convert(surfaceConfig.format);
	auto depthAttachmentFormat = Coral::Vulkan::convert(surfaceConfig.depthFormat.value_or(Coral::PixelFormat::DEPTH24_STENCIL8));
	
	VkPipelineRenderingCreateInfo renderingCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	renderingCreateInfo.colorAttachmentCount    = 1;
	renderingCreateInfo.pColorAttachmentFormats = &colorAttachmentFormat;
	renderingCreateInfo.depthAttachmentFormat   = VK_FORMAT_UNDEFINED;
	renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	if (surfaceConfig.depthFormat)
	{
		renderingCreateInfo.depthAttachmentFormat = Coral::Vulkan::convert(*surfaceConfig.depthFormat);

		if (surfaceConfig.depthFormat == Coral::PixelFormat::DEPTH24_STENCIL8)
		{
			renderingCreateInfo.stencilAttachmentFormat = depthAttachmentFormat;
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
	vkInitInfo.MinImageCount               = surface->config().swapchainImageCount;
	vkInitInfo.ImageCount                  = surface->getSwapchainImageCount();
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
