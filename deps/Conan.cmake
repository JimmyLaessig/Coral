# Download conan.cmake automatically
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/develop/conan.cmake" "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

set(CORAL_CONAN_PACKAGES
    vk-bootstrap/0.7
    volk/1.3.268.0
    vulkan-headers/1.3.268.0
    vulkan-memory-allocator/cci.20231120)

set(CORAL_TOOLS_CONAN_PACKAGES
    cli11/2.4.2
    shaderc/2023.6)

set(CORAL_TEST_CONAN_PACKAGES
    catch2/3.7.1)

set(CORAL_EXAMPLES_CONAN_PACKAGES
    glfw/3.3.8
    glm/cci.20220420
    stb/cci.20240531)

if (CORAL_BUILD_TOOLS OR CORAL_BUILD_EXAMPLES)
    list(APPEND CORAL_CONAN_PACKAGES ${CORAL_TOOLS_CONAN_PACKAGES})
endif()

if (CORAL_BUILD_TESTS)
    list(APPEND CORAL_CONAN_PACKAGES ${CORAL_TEST_CONAN_PACKAGES})
endif()

if (CORAL_BUILD_EXAMPLES)
    list(APPEND CORAL_CONAN_PACKAGES ${CORAL_TEST_CONAN_PACKAGES})
endif()

conan_cmake_configure(
    REQUIRES
        ${CORAL_CONAN_PACKAGES}
  GENERATORS
        cmake_find_package_multi)

foreach(TYPE ${CMAKE_CONFIGURATION_TYPES})
    conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})
    conan_cmake_install(PATH_OR_REFERENCE .
                        BUILD missing
                        REMOTE conancenter
                        SETTINGS ${settings})
endforeach()

