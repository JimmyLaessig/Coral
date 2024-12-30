###############################################################################
# This CMakeLists.txt contains the build and ctest configuration of the 
# Unit tests.
###############################################################################

set(UNIT_TESTS_TARGET_NAME UnitTests)

###############################################################################
# Create the Unit tests executable
###############################################################################

add_executable(${UNIT_TESTS_TARGET_NAME})

# The source location of the Log20 tests
set(UNIT_TESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Definition of all source files
set(UNIT_TESTS_COMMON_SOURCES
    ${UNIT_TESTS_SOURCE_DIR}/Common/TypeNameTests.cpp
    ${UNIT_TESTS_SOURCE_DIR}/Common/VisitorTests.cpp
)

# Add the test source files
target_sources(${UNIT_TESTS_TARGET_NAME} 
    PRIVATE ${UNIT_TESTS_COMMON_SOURCES})

###############################################################################
# Link the test executable
###############################################################################

find_package(Catch2 REQUIRED)

target_link_libraries(${UNIT_TESTS_TARGET_NAME}
    PRIVATE
        Common
        Catch2::Catch2WithMain
)

# Depending whether or not the library was built as static or shared, copy the
# DLL to the binary directory of the excutable so that it can be found at 
# runtime.
#if (LOG20_BUILD_SHARED_LIBRARY)
#    get_target_property(LOG20_BINARY_DIR Log20 BINARY_DIR)
#    if (MSVC)
#        set(LOG20_BINARY_FILES "${LOG20_BINARY_DIR}/$<CONFIG>/")
#    else()
#        set(LOG20_BINARY_FILES "${LOG20_BINARY_DIR}/$<CONFIG>/")
#        endif()
#    
#    add_custom_command(TARGET ${LOG20_TEST_TARGET_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory ${LOG20_BINARY_FILES} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
#endif()

###############################################################################
# Setup CTest
###############################################################################

#include(Catch)
include(CTest)
catch_discover_tests(${UNIT_TESTS_TARGET_NAME} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>)
