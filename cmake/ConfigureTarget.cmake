#
#
#
function(coral_configure_target TARGET_NAME)

get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)

###############################################################################
# Set target properties
###############################################################################

# CMake requires the language standard to be specified as compile feature
# when a target provides C++20 modules and the target will be installed 
if (${TARGET_TYPE} STREQUAL "INTERFACE_LIBRARY")
    target_compile_features(${TARGET_NAME} INTERFACE cxx_std_23)
else()
    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
endif()

# The visual studio compiler creates a .pdb files containing the debug 
# information of the library. Setting the following property ensures the
# correct naming and output directory of the .pdb file.
if(MSVC)
    set_target_properties(${TARGET_NAME} PROPERTIES
        COMPILE_PDB_NAME ${TARGET_NAME}
        COMPILE_PDB_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
endif()

###############################################################################
# Install the target
###############################################################################

# GNUInstallDirs contains a well-established directory structure definition for
# cpp libraries
include(GNUInstallDirs)

# Install the .pdb file in debug builds (MSVC only)
if(MSVC)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${TARGET_NAME}.pdb"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            CONFIGURATIONS Debug RelWithDebInfo
            OPTIONAL)
endif()

install(TARGETS ${TARGET_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

endfunction(coral_configure_target)