include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm)

