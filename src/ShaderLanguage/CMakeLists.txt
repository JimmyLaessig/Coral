#######################################################################################################################
# This file contains the build configuration for the Coral Shader Language (CSL) library.
#######################################################################################################################

set(CORALSHADERLANGUAGE_TARGET_NAME CoralShaderLanguage)

#######################################################################################################################
# Build the library as static or shared
#######################################################################################################################

option(CSL_BUILD_SHARED_LIBRARY "Build and link ${CORALSHADERLANGUAGE_TARGET_NAME} as a shared library" OFF)

if (CSL_BUILD_SHARED_LIBRARY)
    set(CSL_TARGET_TYPE SHARED)
else()
    set(CSL_TARGET_TYPE STATIC)
endif()

#######################################################################################################################
# Create the CSL library
#######################################################################################################################

add_library(${CORALSHADERLANGUAGE_TARGET_NAME} ${CSL_TARGET_TYPE})

#######################################################################################################################
# Add the source files to the library
#######################################################################################################################

set(CSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral)

set(CSL_SHADERLANGUAGE_INCLUDE_DIR ${CSL_INCLUDE_DIR}/ShaderLanguage)

set(CSL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CSL_SHADERLANGUAGE_HEADERS
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/ShaderModule.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Compiler.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/CompilerGLSL.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/CompilerSPV.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/ShaderGraph.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Attribute.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/ControlFlow.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Functions.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Matrix.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Parameter.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Sampler.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Scalar.hpp
    ${CSL_SHADERLANGUAGE_INCLUDE_DIR}/Vector.hpp)

set(CSL_HEADER
    ${CSL_INCLUDE_DIR}/ShaderLanguage.hpp)

set(CSL_SOURCES
    ${CSL_SOURCE_DIR}/Visitor.hpp
    ${CSL_SOURCE_DIR}/CompilerGLSL.cpp
    ${CSL_SOURCE_DIR}/CompilerSPV.cpp
    ${CSL_SOURCE_DIR}/ShaderModule.cpp)

#######################################################################################################################
# Link the CSL library with the following libraries
#######################################################################################################################

coral_configure_target(${CORALSHADERLANGUAGE_TARGET_NAME}
    PUBLIC_HEADERS 
        ${CSL_SHADERGRAPH_HEADERS}
        ${CSL_SHADERLANGUAGE_HEADERS}
        ${CSL_HEADERS}
    SOURCES 
        ${CSL_SOURCES}
    PUBLIC_DEPENDENCIES
        Coral
    PRIVATE_DEPENDENCIES
        glslang
        glslang-default-resource-limits
    )

#######################################################################################################################
# Organize files in the IDE
#######################################################################################################################

source_group("include/Coral" FILES ${CSL_HEADER})
source_group("include/Coral/ShaderLanguage" FILES ${CSL_SHADERLANGUAGE_HEADERS})
