include(vulkan-headers.cmake)
include(spirv-reflect.cmake)
include(vk-bootstrap.cmake)
include(vma.cmake)
include(volk.cmake)

include(cli11.cmake)
include(glslang.cmake)

include(catch2.cmake)
include(glfw.cmake)
include(glm.cmake)
include(stb.cmake)
include(imgui.cmake)

# Put all third-party dependencies into a separate folders to cleanup the IDE project explorer.

function (get_all_targets result_var directory)
    get_property(targets DIRECTORY ${directory} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(sub_directories DIRECTORY ${directory} PROPERTY SUBDIRECTORIES)

    foreach(sub_directory ${sub_directories})
        get_all_targets(sub_directory_targets ${sub_directory})
        list(APPEND targets ${sub_directory_targets})
    endforeach()

    set(${result_var} ${targets} PARENT_SCOPE)
endfunction()

get_all_targets(all_targets ${CMAKE_CURRENT_SOURCE_DIR})

foreach (target ${all_targets})
    get_target_property(folder ${target} FOLDER)
   
    if (${folder} STREQUAL "folder-NOTFOUND")
        set(new_folder "deps")
    else()
        set(new_folder "deps/${folder}")
    endif()
    set_target_properties(${target} PROPERTIES FOLDER "${new_folder}")
    set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endforeach()
