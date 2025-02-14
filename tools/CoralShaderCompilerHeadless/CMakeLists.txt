###############################################################################
# The name of the Coral shader compiler command-line executable
###############################################################################

set(CORAL_SHADER_COMPILER_HEADLESS_TARGET_NAME CoralShaderCompilerHeadless)

set(CORAL_SHADER_COMPILER_HEADLESS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

set(CORAL_SHADER_COMPILER_HEADLESS_CMAKE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoralShaderCompiler.cmake.in)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("src/" FILES  ${CORAL_SHADER_COMPILER_HEADLESS_SOURCES})
source_group("cmake/" FILES ${CORAL_SHADER_COMPILER_HEADLESS_CMAKE_FILE})

###############################################################################
# Add the Coral shader compiler executable
###############################################################################

add_executable(${CORAL_SHADER_COMPILER_HEADLESS_TARGET_NAME})

###############################################################################
# Configure the target
###############################################################################

coral_configure_target(${CORAL_SHADER_COMPILER_HEADLESS_TARGET_NAME}
    SOURCES ${CORAL_SHADER_COMPILER_HEADLESS_SOURCES}
    PRIVATE_DEPENDENCIES
        CoralShaderCompiler
        CLI11::CLI11)

###############################################################################
# Create the CMake shader compiler command
###############################################################################

set(CORAL_SHADER_COMPILER_HEADLESS_EXECUTABLE $<TARGET_FILE:${CORAL_SHADER_COMPILER_HEADLESS_TARGET_NAME}>)

configure_file(${CORAL_SHADER_COMPILER_HEADLESS_CMAKE_FILE} ${CMAKE_BINARY_DIR}/CoralShaderCompiler.cmake @ONLY)

include(${CMAKE_BINARY_DIR}/CoralShaderCompiler.cmake)
