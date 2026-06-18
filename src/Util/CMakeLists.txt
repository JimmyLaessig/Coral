#######################################################################################################################
# This file contains the build configuration for the Coral Util library.
#######################################################################################################################

set(TARGET_NAME CoralUtil)

#######################################################################################################################
# Create the Util library
#######################################################################################################################

add_library(${TARGET_NAME} INTERFACE)

#######################################################################################################################
# Add the source files to the library
#######################################################################################################################

set(PUBLIC_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/Coral/Util)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(PUBLIC_HEADERS
    ${PUBLIC_HEADER_DIR}/RAII.hpp
    ${PUBLIC_HEADER_DIR}/UniformBlockBuilder.hpp)

#######################################################################################################################
# Link the CSL library with the following libraries
#######################################################################################################################

target_sources(${TARGET_NAME} INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    FILES ${PUBLIC_HEADERS}
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(${TARGET_NAME} INTERFACE 
    Coral)

coral_configure_target(${TARGET_NAME})

#######################################################################################################################
# Organize files in the IDE
#######################################################################################################################

source_group("include/Coral/Util" FILES ${HEADERS})
