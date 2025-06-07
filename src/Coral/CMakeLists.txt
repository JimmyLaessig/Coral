###############################################################################
# This file contains the build configuration for the Coral library
###############################################################################

# The name of the library
set(CORAL_TARGET_NAME Coral)

###############################################################################
# Build the library as static or shared
###############################################################################

option(CORAL_BUILD_SHARED_LIBRARY "Build and link ${CORAL_TARGET_NAME} as a shared library" OFF)

if (CORAL_BUILD_SHARED_LIBRARY)
    set(CORAL_TARGET_TYPE SHARED)
else()
    set(CORAL_TARGET_TYPE STATIC)
endif()

###############################################################################
# Create the Coral library
###############################################################################

add_library(${CORAL_TARGET_NAME} ${CORAL_TARGET_TYPE})

###############################################################################
# Add the source files to the library
###############################################################################

# Location of the public Coral library include files
set(CORAL_PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/public/include/${CORAL_TARGET_NAME})
set(CORAL_PRIVATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/private/include/${CORAL_TARGET_NAME})

# Location of the Coral library source files
set(CORAL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/private/src)

# Definition of all public module files of the Coral library
set(CORAL_PUBLIC_INCLUDES
    ${CORAL_PUBLIC_INCLUDE_DIR}/Buffer.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/BufferView.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/CommandBuffer.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/CommandQueue.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Context.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Coral.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Fence.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Framebuffer.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Image.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/PipelineState.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Sampler.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Semaphore.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/ShaderModule.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Swapchain.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Types.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/UniformBlockBuilder.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/ImGui_Impl_Coral.hpp)

# Definition of all source files of the Coral library
set(CORAL_SOURCES
    ${CORAL_SOURCE_DIR}/BufferPool.cpp
    ${CORAL_SOURCE_DIR}/Coral.cpp
    ${CORAL_SOURCE_DIR}/UniformBlockBuilder.cpp)

set(CORAL_INCLUDES
    ${CORAL_PRIVATE_INCLUDE_DIR}/BufferPool.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/ContextBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/CommandQueueBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Visitor.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/BufferBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/BufferViewBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/CommandBufferBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/CommandQueueBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/ContextBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/FenceBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/FramebufferBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/ImageBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/PipelineStateBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/SamplerBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/SemaphoreBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/ShaderModuleBase.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Finally.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/SwapchainBase.hpp)

# Definition of all private vulkan header files of the Coral library
set(CORAL_VULKAN_INCLUDES
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/BufferImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/BufferViewImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/CommandBufferImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/CommandQueueImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/ContextImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/FenceImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/FramebufferImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/ImageImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/PipelineStateImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/SamplerImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/SemaphoreImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/ShaderModuleImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/SwapchainImpl.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/Vulkan.hpp
    ${CORAL_PRIVATE_INCLUDE_DIR}/Vulkan/VulkanFormat.hpp)

# Definition of all Vulkan source files of the Coral library
set(CORAL_VULKAN_SOURCES
    ${CORAL_SOURCE_DIR}/Vulkan/BufferImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/BufferViewImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/CommandBufferImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/CommandQueueImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ContextImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/FenceImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/FramebufferImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ImageImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/PipelineStateImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/SamplerImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/SemaphoreImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ShaderModuleImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/SwapchainImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ImGui_Impl_Coral.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp)

if(WIN32)
    list(APPEND CORAL_VULKAN_SOURCES
         ${CORAL_SOURCE_DIR}/Vulkan/VulkanWin32.cpp)
         else()
    list(APPEND CORAL_VULKAN_SOURCES
         ${CORAL_SOURCE_DIR}/Vulkan/VulkanLinux.cpp)
endif()

set(CORAL_IMGUI_INCLUDES
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
)

###############################################################################
# Link the Coral library with the following libraries
###############################################################################

coral_configure_target(${CORAL_TARGET_NAME}
     PUBLIC_HEADERS 
        ${CORAL_PUBLIC_INCLUDES}
     PRIVATE_HEADERS 
        ${CORAL_INCLUDES}
        ${CORAL_VULKAN_INCLUDES} 
     SOURCES 
        ${CORAL_SOURCES}
        ${CORAL_VULKAN_SOURCES}
     PRIVATE_DEPENDENCIES
        spirv-reflect-static
        vk-bootstrap::vk-bootstrap
        volk_headers
        VulkanMemoryAllocator
        Vulkan::Headers
    )

target_sources(${CORAL_TARGET_NAME} PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    BASE_DIRS ${imgui_SOURCE_DIR}
    FILES ${CORAL_IMGUI_INCLUDES})

target_compile_definitions(${CORAL_TARGET_NAME} PRIVATE
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_USE_VOLK)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/public/${CORAL_TARGET_NAME}" FILES ${CORAL_PUBLIC_INCLUDES} ${CORAL_EXPORT_HEADER})
source_group("src/${CORAL_TARGET_NAME}" FILES ${CORAL_SOURCES} ${CORAL_INCLUDES})
source_group("src/${CORAL_TARGET_NAME}/Vulkan" FILES ${CORAL_VULKAN_INCLUDES} ${CORAL_VULKAN_SOURCES})
