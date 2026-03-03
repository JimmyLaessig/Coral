#######################################################################################################################
# This file contains the build configuration for the Coral Util library.
#######################################################################################################################

set(TARGET_NAME CoralUtil)

#######################################################################################################################
# Create the Util library
#######################################################################################################################

add_library(${TARGET_NAME} STATIC)

#######################################################################################################################
# Add the source files to the library
#######################################################################################################################

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral/Util)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(HEADERS
    ${INCLUDE_DIR}/RAII.hpp
    ${INCLUDE_DIR}/UniformBlockBuilder.hpp)

set(SOURCES
    ${SOURCE_DIR}/UniformBlockBuilder.cpp)

#######################################################################################################################
# Link the CSL library with the following libraries
#######################################################################################################################

coral_configure_target(${TARGET_NAME}
    PUBLIC_HEADERS 
        ${HEADERS}
    SOURCES 
        ${SOURCES}
    PUBLIC_DEPENDENCIES
        Coral
    PRIVATE_DEPENDENCIES
        glslang
        glslang-default-resource-limits
    )

#######################################################################################################################
# Organize files in the IDE
#######################################################################################################################

source_group("include/Coral/Util" FILES ${HEADERS})
source_group("src" FILES ${SOURCES})