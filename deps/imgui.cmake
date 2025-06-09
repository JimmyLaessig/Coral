include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b-docking
)

FETCHContent_MakeAvailable(imgui)

set(imgui_SOURCE_DIR "${imgui_SOURCE_DIR}" CACHE STRING "Path to imgui source directory" FORCE)
