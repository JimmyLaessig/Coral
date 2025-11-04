#ifndef CORAL_IMGUI_HPP
#define CORAL_IMGUI_HPP

#include <Coral/System.hpp>
#include <Coral/CoralFwd.hpp>
#include <Coral/Framebuffer.hpp>

#include <imgui.h>

struct ImGui_ImplCoral_InitInfo
{
    Coral::Context* context{ nullptr };
    Coral::FramebufferSignature framebufferSignature{};
    uint32_t swapchainImageCount{ 0 };
};

bool CORAL_API ImGui_ImplCoral_Init(ImGui_ImplCoral_InitInfo* initInfo);

bool CORAL_API ImGui_ImplCoral_CreateFontsTexture();

void CORAL_API ImGui_ImplCoral_DestroyFontsTexture();

void CORAL_API ImGui_ImplCoral_RenderDrawData(ImDrawData* drawData, Coral::CommandBuffer* commandBuffer);

void CORAL_API ImGui_ImplCoral_NewFrame();

void CORAL_API ImGui_ImplCoral_Shutdown();

#endif // !CORAL_IMGUIIMPLCORAL_HPP
