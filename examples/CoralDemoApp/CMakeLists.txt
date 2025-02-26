###############################################################################
# Name of the Coral Demo app
###############################################################################

set(CORAL_DEMO_APP_TARGET_NAME CoralDemoApp)

set(CORAL_DEMO_APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CORAL_DEMO_APP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

###############################################################################
# Create the executable
###############################################################################

add_executable(${CORAL_DEMO_APP_TARGET_NAME})

###############################################################################
# Add the target source files
###############################################################################

set (CORAL_DEMO_APP_INCLUDES 
    ${CORAL_RESOURCE_DIRECTORY}/include/Cube.hpp 
    ${CORAL_DEMO_APP_INCLUDE_DIR}/TexturedWithLightingShader.h)

set (CORAL_DEMO_APP_SOURCES ${CORAL_DEMO_APP_SRC_DIR}/main.cpp)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include" FILES ${CORAL_DEMO_APP_INCLUDES})
source_group("src" FILES ${CORAL_DEMO_APP_SOURCES})

###############################################################################
# Configure the target
###############################################################################

coral_configure_target(${CORAL_DEMO_APP_TARGET_NAME}
    SOURCES
        ${CORAL_DEMO_APP_SOURCES}
    PRIVATE_HEADERS
        ${CORAL_DEMO_APP_INCLUDES}
    PRIVATE_DEPENDENCIES
        glm::glm
        glfw
        stb
        Coral
        CSL)

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