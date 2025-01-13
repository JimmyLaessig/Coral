# The name of the CoralShaderCompiler example app executable
set(CORAL_SHADER_COMPILER_TARGET_NAME CoralShaderCompiler)

# The source location of the CoralShaderCompiler example app executable
set(CORAL_SHADER_COMPILER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CORAL_SHADER_COMPILER_PUBLIC_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral/ShaderCompiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral/ShaderIncluder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral/HeaderGenerator.h)

set(CORAL_SHADER_COMPILER_SOURCES
    ${CORAL_SHADER_COMPILER_SRC_DIR}/ShaderCompiler.cpp
    ${CORAL_SHADER_COMPILER_SRC_DIR}/ShaderIncluder.cpp
    ${CORAL_SHADER_COMPILER_SRC_DIR}/HeaderGenerator.cpp)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/Coral/" FILES  ${CORAL_SHADER_COMPILER_PUBLIC_INCLUDES})
source_group("src/" FILES  ${CORAL_SHADER_COMPILER_SOURCES})
source_group("cmake/" FILES ${CORAL_SHADER_COMPILER_CMAKE_FILE})

###############################################################################
# Create the Shader Compiler library
###############################################################################

add_library(${CORAL_SHADER_COMPILER_TARGET_NAME} STATIC)

#find_package(shaderc)

coral_configure_target(${CORAL_SHADER_COMPILER_TARGET_NAME}
          PUBLIC_HEADERS
            ${CORAL_SHADER_COMPILER_PUBLIC_INCLUDES}
          SOURCES
            ${CORAL_SHADER_COMPILER_SOURCES}
        PRIVATE_DEPENDENCIES
            shaderc)
