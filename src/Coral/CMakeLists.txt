###############################################################################
# This file contains the build configuration for the Coral library
###############################################################################

# The name of the library
set(TARGET_NAME Coral)

###############################################################################
# Build the library as static or shared
###############################################################################

option(CORAL_BUILD_SHARED_LIBRARY "Build and link ${TARGET_NAME} as a shared library" OFF)

if (CORAL_BUILD_SHARED_LIBRARY)
    set(TARGET_TYPE SHARED)
else()
    set(TARGET_TYPE STATIC)
endif()

string(TOUPPER "${TARGET_NAME}" TARGET_NAME_UPPERCASE)

set(PUBLIC_HEADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/${TARGET_NAME})
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(GENERATED_HEADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include/${TARGET_NAME})

###############################################################################
# Create the Coral library
###############################################################################

add_library(${TARGET_NAME} ${TARGET_TYPE})

###############################################################################
# Definition of all public header files of the Coral library
###############################################################################

set(PUBLIC_HEADERS
    ${PUBLIC_HEADER_DIR}/Buffer.h
    ${PUBLIC_HEADER_DIR}/CommandBuffer.h
    ${PUBLIC_HEADER_DIR}/CommandQueue.h
    ${PUBLIC_HEADER_DIR}/Context.h
    ${PUBLIC_HEADER_DIR}/Coral.h
    ${PUBLIC_HEADER_DIR}/Core.h
    ${PUBLIC_HEADER_DIR}/Fence.h
    ${PUBLIC_HEADER_DIR}/Framebuffer.h
    ${PUBLIC_HEADER_DIR}/Image.h
    ${PUBLIC_HEADER_DIR}/PipelineState.h
    ${PUBLIC_HEADER_DIR}/Sampler.h
    ${PUBLIC_HEADER_DIR}/Semaphore.h
    ${PUBLIC_HEADER_DIR}/ShaderModule.h
    ${PUBLIC_HEADER_DIR}/Swapchain.h
    ${PUBLIC_HEADER_DIR}/ImGui_Impl_Coral.h)

###############################################################################
# Definition of all source files of the Coral library
###############################################################################

set(SOURCES
    ${SOURCE_DIR}/BufferPool.cpp
    ${SOURCE_DIR}/Core.cpp
    ${SOURCE_DIR}/Buffer.cpp
    ${SOURCE_DIR}/CommandBuffer.cpp
    ${SOURCE_DIR}/CommandQueue.cpp
    ${SOURCE_DIR}/Context.cpp
    ${SOURCE_DIR}/Fence.cpp
    ${SOURCE_DIR}/Framebuffer.cpp
    ${SOURCE_DIR}/Image.cpp
    ${SOURCE_DIR}/PipelineState.cpp
    ${SOURCE_DIR}/Sampler.cpp
    ${SOURCE_DIR}/Semaphore.cpp
    ${SOURCE_DIR}/ShaderModule.cpp
    ${SOURCE_DIR}/Swapchain.cpp
    ${SOURCE_DIR}/ImGui_Impl_Coral.cpp)

###############################################################################
# Definition of all private header files of the Coral library
###############################################################################

set(PRIVATE_HEADERS
    ${SOURCE_DIR}/BufferPool.hpp
    ${SOURCE_DIR}/Visitor.hpp
    ${SOURCE_DIR}/Finally.hpp
    ${SOURCE_DIR}/Buffer.hpp
    ${SOURCE_DIR}/CommandBuffer.hpp
    ${SOURCE_DIR}/CommandQueue.hpp
    ${SOURCE_DIR}/Context.hpp
    ${SOURCE_DIR}/Coral.hpp
    ${SOURCE_DIR}/CoralFwd.hpp
    ${SOURCE_DIR}/Fence.hpp
    ${SOURCE_DIR}/Framebuffer.hpp
    ${SOURCE_DIR}/Image.hpp
    ${SOURCE_DIR}/PipelineState.hpp
    ${SOURCE_DIR}/Sampler.hpp
    ${SOURCE_DIR}/Semaphore.hpp
    ${SOURCE_DIR}/ShaderModule.hpp
    ${SOURCE_DIR}/Swapchain.hpp)

###############################################################################
# Definition of all private Vulkan header files of the Coral library
###############################################################################

set(VULKAN_HEADERS
    ${SOURCE_DIR}/Vulkan/Fwd.hpp
    ${SOURCE_DIR}/Vulkan/BufferImpl.hpp
    ${SOURCE_DIR}/Vulkan/Resource.hpp
    ${SOURCE_DIR}/Vulkan/CommandBufferImpl.hpp
    ${SOURCE_DIR}/Vulkan/CommandQueueImpl.hpp
    ${SOURCE_DIR}/Vulkan/ContextImpl.hpp
    ${SOURCE_DIR}/Vulkan/FenceImpl.hpp
    ${SOURCE_DIR}/Vulkan/FramebufferImpl.hpp
    ${SOURCE_DIR}/Vulkan/ImageImpl.hpp
    ${SOURCE_DIR}/Vulkan/PipelineStateImpl.hpp
    ${SOURCE_DIR}/Vulkan/SamplerImpl.hpp
    ${SOURCE_DIR}/Vulkan/SemaphoreImpl.hpp
    ${SOURCE_DIR}/Vulkan/ShaderModuleImpl.hpp
    ${SOURCE_DIR}/Vulkan/SwapchainImpl.hpp
    ${SOURCE_DIR}/Vulkan/Vulkan.hpp
    ${SOURCE_DIR}/Vulkan/VulkanFormat.hpp
    ${SOURCE_DIR}/Vulkan/ImGuiImpl.hpp)

###############################################################################
# Definition of all Vulkan source files of the Coral library
###############################################################################

set(VULKAN_SOURCES
    ${SOURCE_DIR}/Vulkan/Resource.cpp
    ${SOURCE_DIR}/Vulkan/BufferImpl.cpp
    ${SOURCE_DIR}/Vulkan/CommandBufferImpl.cpp
    ${SOURCE_DIR}/Vulkan/CommandQueueImpl.cpp
    ${SOURCE_DIR}/Vulkan/ContextImpl.cpp
    ${SOURCE_DIR}/Vulkan/FenceImpl.cpp
    ${SOURCE_DIR}/Vulkan/FramebufferImpl.cpp
    ${SOURCE_DIR}/Vulkan/ImageImpl.cpp
    ${SOURCE_DIR}/Vulkan/PipelineStateImpl.cpp
    ${SOURCE_DIR}/Vulkan/SamplerImpl.cpp
    ${SOURCE_DIR}/Vulkan/SemaphoreImpl.cpp
    ${SOURCE_DIR}/Vulkan/ShaderModuleImpl.cpp
    ${SOURCE_DIR}/Vulkan/SwapchainImpl.cpp
    ${SOURCE_DIR}/Vulkan/ImGuiImpl.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

if(WIN32)
    list(APPEND VULKAN_SOURCES
         ${SOURCE_DIR}/Vulkan/VulkanWin32.cpp)
         else()
    list(APPEND VULKAN_SOURCES
         ${SOURCE_DIR}/Vulkan/VulkanLinux.cpp)
endif()

###############################################################################
# Generate the export header
###############################################################################

# The GenerateExportHeader module contains the function definition for 
# generate_export_header()
include(GenerateExportHeader)

set(EXPORT_HEADER "${GENERATED_HEADER_DIR}/Export.h")

# The generate_export_header function is used to generate a file containing
# suitable preprocessor for the EXPORT macros to be used in library classes.
# The header is automatically populated with the proper preprocessor defines
# based on the platform and build type (shared, static). 
# (https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html)
generate_export_header(${TARGET_NAME}
    EXPORT_FILE_NAME ${EXPORT_HEADER}
    EXPORT_MACRO_NAME ${TARGET_NAME_UPPERCASE}_API)

if (${TARGET_TYPE} STREQUAL "STATIC")
    target_compile_definitions(${TARGET_NAME} PUBLIC "${TARGET_NAME_UPPERCASE}_STATIC_DEFINE")
endif()

list(APPEND PUBLIC_HEADERS ${EXPORT_HEADER})

###############################################################################
# Generate the export header
###############################################################################

set(CORAL_IMGUI_INCLUDES
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
)

target_sources(${TARGET_NAME} PUBLIC
    FILE_SET public_headers
    TYPE HEADERS
    FILES ${PUBLIC_HEADERS}
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" 
              "${CMAKE_CURRENT_BINARY_DIR}/generated/include")

target_sources(${TARGET_NAME} PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    FILES ${PRIVATE_HEADERS} ${VULKAN_HEADERS}
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(${TARGET_NAME} PRIVATE
    ${SOURCES} 
    ${VULKAN_SOURCES})

target_sources(${TARGET_NAME} PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS ${imgui_SOURCE_DIR}
    FILES ${CORAL_IMGUI_INCLUDES})

###############################################################################
# Link the Coral library with the following libraries
###############################################################################

target_link_libraries(${TARGET_NAME} PRIVATE
    spirv-reflect-static
    vk-bootstrap::vk-bootstrap
    volk_headers
    VulkanMemoryAllocator
    Vulkan::Headers)

###############################################################################
# Coral-specifc cmake configuration
###############################################################################

coral_configure_target(${TARGET_NAME})

target_compile_definitions(${TARGET_NAME} PRIVATE
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_USE_VOLK)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/${TARGET_NAME}" FILES ${PUBLIC_HEADERS})
source_group("include" FILES ${PRIVATE_HEADERS})
source_group("src" FILES ${SOURCES})
source_group("src/Vulkan" FILES ${VULKAN_HEADERS} ${VULKAN_SOURCES})
