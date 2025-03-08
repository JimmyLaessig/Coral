include(FetchContent)

FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG 15.1.0
  PATCH_COMMAND python ./update_glslang_sources.py)

set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glslang)
