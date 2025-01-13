###############################################################################
# Name of the Coral Demo app
###############################################################################

set(CORAL_DEMO_APP_TARGET_NAME CoralDemoApp)

set(CORAL_DEMO_APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CORAL_DEMO_APP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CORAL_DEMO_APP_SHADER_DIR ${CORAL_DEMO_APP_SRC_DIR}/Shaders)
set(CORAL_DEMO_APP_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CORAL_DEMO_APP_TARGET_NAME}/generated/include)

###############################################################################
# Create the executable
###############################################################################

add_executable(${CORAL_DEMO_APP_TARGET_NAME})

#find_package(glm REQUIRED)
#find_package(glfw3 REQUIRED)
#find_package(stb REQUIRED)

###############################################################################
# Add the target source files
###############################################################################

set (CORAL_DEMO_APP_INCLUDES ${CORAL_RESOURCE_DIRECTORY}/include/Cube.hpp)

set (CORAL_DEMO_APP_SOURCES ${CORAL_DEMO_APP_SRC_DIR}/main.cpp)

###############################################################################
# Add the target shader files
###############################################################################

target_compile_shader(${CORAL_DEMO_APP_TARGET_NAME} 
    NAME UniformColorVertexShader
    NAMESPACE Shaders
    INPUT ${CORAL_DEMO_APP_SHADER_DIR}/UniformColor.hlsl
    OUTPUT ${CORAL_DEMO_APP_GENERATED_INCLUDE_DIR}/Shaders/UniformColorVertexShader.hpp
    STAGE vertex
    ENTRYPOINT VS_main
    LANGUAGE HLSL
    WORKING_DIRECTORY ${CORAL_DEMO_APP_SHADER_DIR})

target_compile_shader(${CORAL_DEMO_APP_TARGET_NAME} 
    NAME UniformColorFragmentShader
    NAMESPACE Shaders
    INPUT ${CORAL_DEMO_APP_SHADER_DIR}/UniformColor.hlsl
    OUTPUT ${CORAL_DEMO_APP_GENERATED_INCLUDE_DIR}/Shaders/UniformColorFragmentShader.hpp
    STAGE fragment
    ENTRYPOINT PS_main
    LANGUAGE HLSL
    WORKING_DIRECTORY ${CORAL_DEMO_APP_SHADER_DIR})

set(CORAL_DEMO_APP_GENERATED_SHADER_HEADERS
    ${CORAL_DEMO_APP_GENERATED_INCLUDE_DIR}/Shaders/UniformColorVertexShader.hpp
    ${CORAL_DEMO_APP_GENERATED_INCLUDE_DIR}/Shaders/UniformColorFragmentShader.hpp)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/generated/Shaders" FILES ${CORAL_DEMO_APP_GENERATED_SHADER_HEADERS})
source_group("include" FILES ${CORAL_DEMO_APP_INCLUDES})
source_group("src" FILES ${CORAL_DEMO_APP_SOURCES})
source_group("Shaders" FILES ${CORAL_DEMO_APP_SHADER_SOURCES})

###############################################################################
# Configure the target
###############################################################################

coral_configure_target(${CORAL_DEMO_APP_TARGET_NAME}
    SOURCES
        ${CORAL_DEMO_APP_SOURCES}
    PRIVATE_HEADERS 
        ${CORAL_DEMO_APP_INCLUDES}
        ${CORAL_DEMO_APP_GENERATED_SHADER_HEADERS}
    PRIVATE_DEPENDENCIES
        glm::glm
        glfw
        stb
        Coral)

###############################################################################
# Copy the resource files
###############################################################################

set(CORAL_DEMO_APP_RESOURCES "${CORAL_RESOURCE_DIRECTORY}/uvtest.png")

add_custom_command(TARGET ${CORAL_DEMO_APP_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources/)

foreach(FILE ${CORAL_DEMO_APP_RESOURCES})
    add_custom_command(TARGET ${CORAL_DEMO_APP_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources/)
endforeach()

###############################################################################
# Copy the shared library files
###############################################################################

get_target_property(CORAL_TARGET_TYPE Coral TYPE)
if(${CORAL_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET ${CORAL_DEMO_APP_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Coral> ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/)
endif()