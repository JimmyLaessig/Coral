include(FetchContent)

FetchContent_Declare(
  shaderc
  GIT_REPOSITORY https://github.com/google/shaderc.git
  GIT_TAG v2024.4
  PATCH_COMMAND git checkout . && git apply --ignore-whitespace "${CMAKE_SOURCE_DIR}/deps/shaderc.patch" && python ./utils/git-sync-deps
)

set(GLSLANG_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(shaderc)
