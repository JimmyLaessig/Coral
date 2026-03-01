
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

CPMAddPackage(
  NAME glslang
  GIT_TAG vulkan-sdk-1.4.309.0
  GITHUB_REPOSITORY KhronosGroup/glslang
  PATCH_COMMAND python ./update_glslang_sources.py
)
