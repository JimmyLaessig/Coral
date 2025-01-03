###############################################################################
# Name of the Coral Unit tests
###############################################################################

set(CORAL_UNIT_TESTS_TARGET_NAME CoralUnitTests)

set(CORAL_UNIT_TESTS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CORAL_UNIT_TESTS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(CORAL_UNIT_TESTS_SHADER_DIR ${CORAL_UNIT_TESTS_SRC_DIR}/Shaders)
set(CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CORAL_UNIT_TESTS_TARGET_NAME}/generated/include)

###############################################################################
# Create the executable
###############################################################################

add_executable(${CORAL_UNIT_TESTS_TARGET_NAME})

###############################################################################
# Add the target source files
###############################################################################

set(CORAL_UNIT_TESTS_INCLUDES
    ${CORAL_RESOURCE_DIRECTORY}/Cube.hpp 
    ${CORAL_UNIT_TESTS_SRC_DIR}/Catch.hpp)

set(CORAL_UNIT_TESTS_SOURCES
    ${CORAL_UNIT_TESTS_SRC_DIR}/Catch.cpp
    ${CORAL_UNIT_TESTS_SRC_DIR}/ContextTests.cpp
    ${CORAL_UNIT_TESTS_SRC_DIR}/BufferTests.cpp
    ${CORAL_UNIT_TESTS_SRC_DIR}/BufferViewTests.cpp)

##############################################################################
# Add the target shader files
###############################################################################

#target_compile_shader(${CORAL_UNIT_TESTS_TARGET_NAME} 
#    NAME UniformColorVertexShader
#    NAMESPACE Shaders
#    INPUT ${CORAL_UNIT_TESTS_SHADER_DIR}/UniformColor.hlsl
#    OUTPUT ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorVertexShader.h
#    STAGE vertex
#    ENTRYPOINT VS_main
#    LANGUAGE HLSL
#    WORKING_DIRECTORY ${CORAL_UNIT_TESTS_SHADER_DIR})

#target_compile_shader(${CORAL_UNIT_TESTS_TARGET_NAME} 
#    NAME UniformColorFragmentShader
#    NAMESPACE Shaders
#    INPUT ${CORAL_UNIT_TESTS_SHADER_DIR}/UniformColor.hlsl
#    OUTPUT ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorFragmentShader.h
#    STAGE fragment
#    ENTRYPOINT PS_main
#    LANGUAGE HLSL
#    WORKING_DIRECTORY ${CORAL_UNIT_TESTS_SHADER_DIR})

#set(CORAL_UNIT_TESTS_GENERATED_SHADER_HEADERS
#    ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorVertexShader.h
#    ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorFragmentShader.h)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/generated/Shaders" FILES ${CORAL_UNIT_TESTS_GENERATED_SHADER_HEADERS})
source_group("include" FILES ${CORAL_UNIT_TESTS_INCLUDES})
source_group("src" FILES ${CORAL_UNIT_TESTS_SOURCES})
source_group("Shaders" FILES ${CORAL_UNIT_TESTS_SHADER_SOURCES})

###############################################################################
# Configure the target
###############################################################################

find_package(glm)
find_package(glfw3)
find_package(stb)
find_package(Catch2)

coral_configure_target(${CORAL_UNIT_TESTS_TARGET_NAME}
    SOURCES
        ${CORAL_UNIT_TESTS_SOURCES}
    PRIVATE_HEADERS 
        ${CORAL_UNIT_TESTS_INCLUDES}
        ${CORAL_UNIT_TESTS_GENERATED_SHADER_HEADERS}
    PRIVATE_DEPENDENCIES
        glm::glm
        glfw::glfw
        stb::stb
        Coral
        Catch2::Catch2WithMain)

###############################################################################
# Copy the resource files
###############################################################################

set(CORAL_UNIT_TESTS_RESOURCES "${CORAL_RESOURCE_DIRECTORY}/uvtest.png")

add_custom_command(TARGET ${CORAL_UNIT_TESTS_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources/)

foreach(FILE ${CORAL_UNIT_TESTS_RESOURCES})
    add_custom_command(TARGET ${CORAL_UNIT_TESTS_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources/)
endforeach()

###############################################################################
# Copy the shared library files
###############################################################################

get_target_property(CORAL_TARGET_TYPE Coral TYPE)
if(${CORAL_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET ${CORAL_UNIT_TESTS_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Coral> ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/)
endif()

###############################################################################
# Configure CTest and Catch
###############################################################################

include(CTest)
include(Catch)
catch_discover_tests(${CORAL_UNIT_TESTS_TARGET_NAME})