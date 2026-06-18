#ifndef CORAL_IMGUI_IMPL_CORAL_H
#define CORAL_IMGUI_IMPL_CORAL_H

#include <Coral/Framebuffer.h>
#include <Coral/CommandBuffer.h>

#include <imgui.h>

typedef struct  
{
    CoContext context;
    CoFramebufferLayout framebufferSignature;
    uint32_t swapchainImageCount;

} ImGui_ImplCoral_InitInfo;

CoResult CORAL_API ImGui_ImplCoral_Init(ImGui_ImplCoral_InitInfo* initInfo);

CoResult CORAL_API ImGui_ImplCoral_CreateFontsTexture(CoContext context);

void CORAL_API ImGui_ImplCoral_DestroyFontsTexture(CoContext context);

void CORAL_API ImGui_ImplCoral_RenderDrawData(CoContext context, ImDrawData* drawData, CoCommandBuffer commandBuffer);

void CORAL_API ImGui_ImplCoral_NewFrame(CoContext context);

void CORAL_API ImGui_ImplCoral_Shutdown(CoContext context);

#endif // !CORAL_IMGUI_IMPL_CORAL_H
