###############################################################################
# This file contains the build configuration for the Coral Shader Language (CSL) library.
###############################################################################

set(CSL_TARGET_NAME CSL)

###############################################################################
# Build the library as static or shared
###############################################################################

option(CSL_BUILD_SHARED_LIBRARY "Build and link ${CSL_TARGET_NAME} as a shared library" OFF)

if (CSL_BUILD_SHARED_LIBRARY)
    set(CSL_TARGET_TYPE SHARED)
else()
    set(CSL_TARGET_TYPE STATIC)
endif()

###############################################################################
# Create the CSL library
###############################################################################

add_library(${CSL_TARGET_NAME} ${CSL_TARGET_TYPE})

###############################################################################
# Add the source files to the library
###############################################################################

# Location of the public Coral library include files
set(CSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral)

set(CSL_SHADERGRAPH_INCLUDE_DIR ${CSL_INCLUDE_DIR}/ShaderGraph)

# Location of the Coral library source files
set(CSL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Definition of all public module files of the Coral library
set(CSL_SHADERGRAPH_HEADERS
    ${CSL_SHADERGRAPH_INCLUDE_DIR}/Compiler.hpp
    ${CSL_SHADERGRAPH_INCLUDE_DIR}/CompilerGLSL.hpp
    ${CSL_SHADERGRAPH_INCLUDE_DIR}/CompilerSPV.hpp
    ${CSL_SHADERGRAPH_INCLUDE_DIR}/ShaderGraph.hpp)

set(CSL_HEADERS
    ${CSL_INCLUDE_DIR}/ShaderLanguage.hpp
    ${CSL_INCLUDE_DIR}/ShaderLanguage/Functions.hpp
    ${CSL_INCLUDE_DIR}/ShaderLanguage/Vector.hpp
    ${CSL_INCLUDE_DIR}/ShaderLanguage/Matrix.hpp
    ${CSL_INCLUDE_DIR}/ShaderLanguage/Scalar.hpp
    ${CSL_INCLUDE_DIR}/ShaderLanguage/Sampler.hpp)


# Definition of all source files of the Coral library
set(CSL_SOURCES
    ${CSL_SOURCE_DIR}/Visitor.hpp
    ${CSL_SOURCE_DIR}/CompilerGLSL.cpp
    ${CSL_SOURCE_DIR}/CompilerSPV.cpp
    ${CSL_SOURCE_DIR}/ShaderGraph.cpp)


###############################################################################
# Link the CSL library with the following libraries
###############################################################################

coral_configure_target(${CSL_TARGET_NAME}
    PUBLIC_HEADERS 
        ${CSL_SHADERGRAPH_HEADERS}
        ${CSL_HEADERS}
    SOURCES 
        ${CSL_SOURCES}
    PUBLIC_DEPENDENCIES
        Coral
    PRIVATE_DEPENDENCIES
        glslang
        glslang-default-resource-limits
    )

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/Coral" FILES ${CSL_HEADERS})
source_group("include/Coral/ShaderGraph" FILES ${CSL_SHADERGRAPH_HEADERS})
source_group("src/Coral/ShaderLanguage" FILES ${CSL_SOURCES})
