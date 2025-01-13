include(FetchContent)

FetchContent_Declare(
  vulkan-headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.3.302
)

set (SHADERC_SKIP_TESTS ON)
FetchContent_MakeAvailable(vulkan-headers)
