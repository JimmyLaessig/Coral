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
set(CORAL_PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/${CORAL_TARGET_NAME})

# Location of the Coral library source files
set(CORAL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Definition of all public module files of the Coral library
set(CORAL_PUBLIC_INCLUDES
    ${CORAL_PUBLIC_INCLUDE_DIR}/Buffer.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/BufferView.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/CommandBuffer.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/CommandQueue.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Context.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Coral.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/DescriptorSet.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Fence.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Framebuffer.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Image.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/PipelineState.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Sampler.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Semaphore.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/ShaderModule.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Surface.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/Types.hpp
    ${CORAL_PUBLIC_INCLUDE_DIR}/UniformBlockBuilder.hpp)

# Definition of all source files of the Coral library
set(CORAL_SOURCES
    ${CORAL_SOURCE_DIR}/BufferPool.cpp
    ${CORAL_SOURCE_DIR}/BufferPool.hpp
    ${CORAL_SOURCE_DIR}/Finally.hpp
    ${CORAL_SOURCE_DIR}/Context.cpp
    ${CORAL_SOURCE_DIR}/PipelineState.cpp
    ${CORAL_SOURCE_DIR}/UniformBlockBuilder.cpp)

set(CORAL_PRIVATE_HEADERS
    ${CORAL_SOURCE_DIR}/Visitor.hpp)

# Definition of all private vulkan header files of the Coral library
set(CORAL_VULKAN_INCLUDES
    ${CORAL_SOURCE_DIR}/Vulkan/BufferImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/BufferViewImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/CommandBufferImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/CommandQueueImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/ContextImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/DescriptorSetImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/DescriptorSetPool.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/FenceImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/FramebufferImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/ImageImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/PipelineStateImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/SamplerImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/SemaphoreImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/ShaderModuleImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/SurfaceImpl.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/Vulkan.hpp
    ${CORAL_SOURCE_DIR}/Vulkan/VulkanFormat.hpp)

# Definition of all Vulkan source files of the Coral library
set(CORAL_VULKAN_SOURCES
    ${CORAL_SOURCE_DIR}/Vulkan/BufferImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/BufferViewImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/CommandBufferImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/CommandQueueImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ContextImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/DescriptorSetImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/DescriptorSetPool.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/FenceImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/FramebufferImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ImageImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/PipelineStateImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/SamplerImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/SemaphoreImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/ShaderModuleImpl.cpp
    ${CORAL_SOURCE_DIR}/Vulkan/SurfaceImpl.cpp)

if(WIN32)
    list(APPEND CORAL_VULKAN_SOURCES
         ${CORAL_SOURCE_DIR}/Vulkan/VulkanWin32.cpp)
         else()
    list(APPEND CORAL_VULKAN_SOURCES
         ${CORAL_SOURCE_DIR}/Vulkan/VulkanLinux.cpp)
         
endif()

###############################################################################
# Link the Coral library with the following libraries
###############################################################################

#find_package(VulkanHeaders CONFIG REQUIRED)
#find_package(volk CONFIG REQUIRED)
#find_package(vk-bootstrap REQUIRED)
#find_package(vulkan-memory-allocator REQUIRED)

#if (CORAL_SKIP_CONAN_DEPENDENCIES)
#    find_package(spirv-reflect REQUIRED)
#endif()

coral_configure_target(${CORAL_TARGET_NAME}
     PUBLIC_HEADERS 
        ${CORAL_PUBLIC_INCLUDES}
     PRIVATE_HEADERS 
        ${CORAL_PRIVATE_HEADERS}
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

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/public/${CORAL_TARGET_NAME}" FILES ${CORAL_PUBLIC_INCLUDES} ${CORAL_EXPORT_HEADER})
source_group("src/${CORAL_TARGET_NAME}" FILES ${CORAL_SOURCES})
source_group("src/${CORAL_TARGET_NAME}/Vulkan" FILES ${CORAL_VULKAN_INCLUDES} ${CORAL_VULKAN_SOURCES})
