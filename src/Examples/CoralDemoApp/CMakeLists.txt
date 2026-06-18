###############################################################################
# Name of the Coral Demo app
###############################################################################

set(TARGET_NAME CoralDemoApp)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(GENERATED_HEADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include)

###############################################################################
# Create the executable
###############################################################################

add_executable(${TARGET_NAME})

###############################################################################
# Add the target source files
###############################################################################

set(HEADERS
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.h
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imgui.h
    ${GENERATED_HEADER_DIR}/Shader_slang.hpp)

set(SOURCES 
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/Shader.slang
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("src" FILES ${SOURCES})

###############################################################################
# Configure the target
###############################################################################

target_embed_files(${TARGET_NAME} PRIVATE
    FILE_SET private_headers
    FILES "${SOURCE_DIR}/Shader.slang"
    OUTPUT_DIR ${GENERATED_HEADER_DIR}
    BASE_DIRS ${GENERATED_HEADER_DIR}
    NAMESPACE Shaders)

coral_configure_target(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE
    ${SOURCES})

target_sources(${TARGET_NAME} PRIVATE
    FILE_SET  private_headers
    TYPE      HEADERS
    BASE_DIRS ${imgui_SOURCE_DIR} ${GENERATED_HEADER_DIR}
    FILES     ${HEADERS})

target_link_libraries(${TARGET_NAME} PRIVATE
    glm::glm
    glfw
    stb
    Coral
    CoralUtil
    CoralResources
    slang)

###############################################################################
# Copy the shared library files
###############################################################################

get_target_property(CORAL_TARGET_TYPE Coral TYPE)
if(${CORAL_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Coral> ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/)
endif()
