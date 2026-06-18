###############################################################################
# Name of the Coral Unit tests
###############################################################################

set(TARGET_NAME CoralUnitTests)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CORAL_UNIT_TESTS_SHADER_DIR ${SOURCE_DIR}/Shaders)
set(CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}/generated/include)

###############################################################################
# Create the executable
###############################################################################

add_executable(${TARGET_NAME})

###############################################################################
# Add the target source files
###############################################################################

set(HEADERS 
    ${SOURCE_DIR}/Catch.hpp)

set(SOURCES
    ${SOURCE_DIR}/Catch.hpp
    ${SOURCE_DIR}/Catch.cpp
    ${SOURCE_DIR}/ContextTests.cpp
    ${SOURCE_DIR}/BufferTests.cpp
    ${SOURCE_DIR}/UniformBlockBuilderTests.cpp)

##############################################################################
# Add the target shader files
###############################################################################

#target_compile_shader(${TARGET_NAME} 
#    NAME UniformColorVertexShader
#    NAMESPACE Shaders
#    INPUT ${CORAL_UNIT_TESTS_SHADER_DIR}/UniformColor.hlsl
#    OUTPUT ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorVertexShader.h
#    STAGE vertex
#    ENTRYPOINT VS_main
#    LANGUAGE HLSL
#    WORKING_DIRECTORY ${CORAL_UNIT_TESTS_SHADER_DIR})

#target_compile_shader(${TARGET_NAME} 
#    NAME UniformColorFragmentShader
#    NAMESPACE Shaders
#    INPUT ${CORAL_UNIT_TESTS_SHADER_DIR}/UniformColor.hlsl
#    OUTPUT ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorFragmentShader.h
#    STAGE fragment
#    ENTRYPOINT PS_main
#    LANGUAGE HLSL
#    WORKING_DIRECTORY ${CORAL_UNIT_TESTS_SHADER_DIR})

#set(CORAL_UNIT_TESTS_GENERATED_SHADER_HEADERS
#    ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorVertexShader.h
#    ${CORAL_UNIT_TESTS_GENERATED_INCLUDE_DIR}/Shaders/UniformColorFragmentShader.h)

###############################################################################
# Organize files in the IDE
###############################################################################

source_group("include/generated/Shaders" FILES ${CORAL_UNIT_TESTS_GENERATED_SHADER_HEADERS})
source_group("src" FILES ${SOURCES})
source_group("Shaders" FILES ${CORAL_UNIT_TESTS_SHADER_SOURCES})

###############################################################################
# Configure the target
###############################################################################

target_sources(${TARGET_NAME} PRIVATE
    FILE_SET private_headers
    TYPE HEADERS
    FILES ${HEADERS}
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

target_link_libraries(${TARGET_NAME} PRIVATE
        glm::glm
        stb
        Coral
        Catch2::Catch2WithMain
        CoralResources)

coral_configure_target(${TARGET_NAME})


###############################################################################
# Copy the resource files
###############################################################################

#set(CORAL_UNIT_TESTS_RESOURCES "${CORAL_RESOURCE_DIRECTORY}/uvtest.png")

#add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources/)

#foreach(FILE ${CORAL_UNIT_TESTS_RESOURCES})
#    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/resources/)
#endforeach()

###############################################################################
# Copy the shared library files
###############################################################################

get_target_property(CORAL_TARGET_TYPE Coral TYPE)
if(${CORAL_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Coral> ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/)
endif()

###############################################################################
# Configure CTest and Catch
###############################################################################

include(CTest)
include(Catch)
catch_discover_tests(${TARGET_NAME})