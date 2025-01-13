cmake_minimum_required(VERSION 3.28)

# The name of the project
set(PROJECT_NAME Coral)

# The version of the Coral project
set(CORAL_VERSION_MAJOR 0)
set(CORAL_VERSION_MINOR 1)
set(CORAL_VERSION_PATCH 0)
set(CORAL_PROJECT_VERSION "${CORAL_VERSION_MAJOR}.${CORAL_VERSION_MINOR}.${CORAL_VERSION_PATCH}")

project(${PROJECT_NAME} VERSION ${CORAL_PROJECT_VERSION})

option(CORAL_BUILD_SHARED_LIBRARY "Build and link ${CORAL_TARGET_NAME} as a shared library" OFF)
option(CORAL_BUILD_TOOLS "Build the Coral tools" ON)
option(CORAL_BUILD_EXAMPLES "Build the Coral examples" ON)
option(CORAL_BUILD_TESTS "Build the Coral tests" ON)
option(CORAL_SKIP_CONAN_DEPENDENCIES "If enabled, skip downloading the Coral dependencies via Conan." OFF)

get_property(generator_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if (generator_is_multi_config)
    set(CORAL_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "The build types of the Coral library (Debug, Release, RelWithDebInfo)" )
    set(CMAKE_CONFIGURATION_TYPES ${CORAL_CONFIGURATION_TYPES})
else()
    set(CORAL_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "The build types of the Coral library (Debug, Release)")
    set(CMAKE_BUILD_TYPE ${CORAL_CONFIGURATION_TYPES})
endif()

include(cmake/ConfigureTarget.cmake)

set(CORAL_RESOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(tools)
if(CORAL_BUILD_EXAMPLES OR CORAL_BUILD_TOOLS)
#    add_subdirectory(tools)
endif()

if(CORAL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(CORAL_BUILD_TESTS)
    add_subdirectory(test)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
