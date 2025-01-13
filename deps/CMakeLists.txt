set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

include(vulkan-headers.cmake)
include(spirv-reflect.cmake)
include(vk-bootstrap.cmake)
include(vma.cmake)
include(volk.cmake)

include(cli11.cmake)
include(shaderc.cmake)

include(glfw.cmake)
include(glm.cmake)
include(stb.cmake)

# To cleanup the project files, put all third-party dependencies into a separate folders

# Helper macro to get all targets of the third-party dependencies
macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()

    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()

# Helper function to get all targets of the third-party dependencies
macro(get_all_targets var)
    set(targets)
    get_all_targets_recursive(targets ${CMAKE_CURRENT_SOURCE_DIR})
    set(${var} ${targets} PARENT_SCOPE)
endmacro()

get_all_targets(all_targets)

foreach (target ${all_targets})
    get_target_property(folder ${target} FOLDER)
   
    if (${folder} STREQUAL "folder-NOTFOUND")
        set(new_folder "deps")
    else()
        set(new_folder "deps/${folder}")
    endif()

    set_target_properties(${target} PROPERTIES FOLDER "${new_folder}")
endforeach()
