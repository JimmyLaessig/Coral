include(FetchContent)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
)

FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)

target_sources(stb PUBLIC 
  FILE_SET public_headers
  TYPE HEADERS
  BASE_DIRS ${stb_SOURCE_DIR}
  FILES ${stb_SOURCE_DIR}/stb_image.h)
