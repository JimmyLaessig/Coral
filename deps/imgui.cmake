CPMAddPackage(
  NAME imgui
  GIT_TAG v1.91.9b-docking
  GITHUB_REPOSITORY ocornut/imgui
)

set(imgui_SOURCE_DIR "${imgui_SOURCE_DIR}" CACHE STRING "Path to imgui source directory" FORCE)
