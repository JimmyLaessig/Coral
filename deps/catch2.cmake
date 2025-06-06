include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.8.0
)

set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)

set(CATCH_CONFIG_CPP11_TO_STRING ON CACHE BOOL "" FORCE)
set(CATCH_CONFIG_CPP17_BYTE ON CACHE BOOL "" FORCE)
set(CATCH_CONFIG_CPP17_OPTIONAL ON CACHE BOOL "" FORCE)
set(CATCH_CONFIG_CPP17_STRING_VIEW ON CACHE BOOL "" FORCE)
set(CATCH_CONFIG_CPP17_UNCAUGHT_EXCEPTIONS ON CACHE BOOL "" FORCE)
set(CATCH_CONFIG_CPP17_VARIANT ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)

target_compile_features(Catch2 PUBLIC cxx_std_17)
target_compile_features(Catch2WithMain PUBLIC cxx_std_17)
