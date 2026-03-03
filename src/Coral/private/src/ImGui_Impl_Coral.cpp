#include <Coral/ImGui_Impl_Coral.h>

#include <Coral/Vulkan/ImGuiImpl.hpp>
#include <Coral/Vulkan/CommandBufferImpl.hpp>
#include <Coral/Vulkan/CommandQueueImpl.hpp>
#include <Coral/Vulkan/ContextImpl.hpp>


CoResult
ImGui_ImplCoral_Init(ImGui_ImplCoral_InitInfo* initInfo)
{
	switch (coContextGetGraphicsAPI(initInfo->context))
	{
		case CO_GRAPHICS_API_VULKAN:
		{
			auto& contextImpl = static_cast<Coral::Vulkan::ContextImpl&>(*initInfo->context->impl);

			return Coral::Vulkan::ImGui_ImplCoral_Init(contextImpl, 
				                                       initInfo->framebufferSignature, 
				                                       initInfo->swapchainImageCount) ? CO_SUCCESS : CO_FAILED;
		}
	}
	return CO_FAILED;
}


CoResult
ImGui_ImplCoral_CreateFontsTexture(CoContext context)
{
	return Coral::Vulkan::ImGui_ImplCoral_CreateFontsTexture() ? CO_SUCCESS : CO_FAILED;
}


void
ImGui_ImplCoral_DestroyFontsTexture(CoContext context)
{
	switch (context->impl->graphicsAPI())
	{
		case CO_GRAPHICS_API_VULKAN:
			Coral::Vulkan::ImGui_ImplCoral_Shutdown();
			break;
	}
}


void
ImGui_ImplCoral_RenderDrawData(CoContext context, ImDrawData* drawData, CoCommandBuffer commandBuffer)
{
	switch (context->impl->graphicsAPI())
	{
		case CO_GRAPHICS_API_VULKAN:
			Coral::Vulkan::ImGui_ImplCoral_RenderDrawData(drawData, *static_cast<Coral::Vulkan::CommandBufferImpl*>(commandBuffer->impl.get()));
			break;
	}
}


void
ImGui_ImplCoral_NewFrame(CoContext context)
{
	switch (context->impl->graphicsAPI())
	{
	case CO_GRAPHICS_API_VULKAN:
		Coral::Vulkan::ImGui_ImplCoral_NewFrame();
		break;
	}
}


void
ImGui_ImplCoral_Shutdown(CoContext context)
{
	switch (context->impl->graphicsAPI())
	{
		case CO_GRAPHICS_API_VULKAN:
			Coral::Vulkan::ImGui_ImplCoral_Shutdown();
			break;
	}
}
