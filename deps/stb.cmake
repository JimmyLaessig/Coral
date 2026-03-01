CPMAddPackage(
  NAME stb
  GITHUB_REPOSITORY nothings/stb
  GIT_TAG f1c79c02822848a9bed4315b12c8c8f3761e1296
)

add_library(stb INTERFACE)

target_sources(stb PUBLIC 
  FILE_SET public_headers
  TYPE HEADERS
  BASE_DIRS ${stb_SOURCE_DIR}
  FILES ${stb_SOURCE_DIR}/stb_image.h)
