#ifndef CORAL_IMGUI_HPP
#define CORAL_IMGUI_HPP

#include <Coral/System.hpp>

#include <Coral/Context.hpp>
#include <Coral/Surface.hpp>

#include <imgui.h>

struct ImGui_ImplCoral_InitInfo
{
    Coral::Context* context{ nullptr };
    Coral::Surface* surface{ nullptr };
};

bool CORAL_API ImGui_ImplCoral_Init(ImGui_ImplCoral_InitInfo* initInfo);

bool CORAL_API ImGui_ImplCoral_CreateFontsTexture();

void CORAL_API ImGui_ImplCoral_DestroyFontsTexture();

void CORAL_API ImGui_ImplCoral_RenderDrawData(ImDrawData* drawData, Coral::CommandBuffer* commandBuffer);

void CORAL_API ImGui_ImplCoral_NewFrame();

void CORAL_API ImGui_ImplCoral_Shutdown();

#endif // !CORAL_IMGUIIMPLCORAL_HPP
