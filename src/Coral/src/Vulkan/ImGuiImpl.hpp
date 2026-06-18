#ifndef CORAL_IMGUI_HPP
#define CORAL_IMGUI_HPP

#include "Fwd.hpp"
#include "Framebuffer.hpp"

#include <imgui.h>

namespace Coral::Vulkan
{

bool ImGui_ImplCoral_Init(Coral::Vulkan::ContextImpl& context, const CoFramebufferLayout& signature, uint32_t swapchainImageCount);

bool ImGui_ImplCoral_CreateFontsTexture();

void ImGui_ImplCoral_DestroyFontsTexture();

void ImGui_ImplCoral_RenderDrawData(ImDrawData* drawData, Coral::Vulkan::CommandBufferImpl& commandBuffer);

void ImGui_ImplCoral_NewFrame();

void ImGui_ImplCoral_Shutdown();

} // namespace Coral::Vulkan

#endif // !CORAL_IMGUIIMPLCORAL_HPP
