#
#
#
function(find_include_directories headers result)
    set(includes "")
    foreach(file IN LISTS ${headers})
        string(FIND ${file} "include" found)

        if (found GREATER_EQUAL 0)
            string(SUBSTRING ${file} 0 ${found} INCLUDE_DIRECTORY)
    
            list(APPEND includes "${INCLUDE_DIRECTORY}include")
        endif()
        
    endforeach()

    list(REMOVE_DUPLICATES includes)

    set(${result} ${includes} PARENT_SCOPE)

endfunction()

#
#
#
function(coral_configure_target TARGET_NAME)

set(multiValueArgs PUBLIC_HEADERS PRIVATE_HEADERS SOURCES PUBLIC_MODULES PRIVATE_MODULES MODULE_SOURCES PUBLIC_DEPENDENCIES PRIVATE_DEPENDENCIES)

cmake_parse_arguments("INPUT" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)

###############################################################################
# Generate the export header
###############################################################################

if (NOT(${TARGET_TYPE} STREQUAL "EXECUTABLE"))
    # The GenerateExportHeader module contains the function definition for 
    # generate_export_header()
    include(GenerateExportHeader)

    string(TOUPPER "${TARGET_NAME}" TARGET_NAME_UPPERCASE)

    set(GENERATED_HEADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include)
    set(EXPORT_HEADER ${GENERATED_HEADER_DIR}/${TARGET_NAME}/System.hpp)

    # The generate_export_header function is used to generate a file containing
    # suitable preprocessor for the EXPORT macros to be used in library classes.
    # The header is automatically populated with the proper preprocessor defines
    # based on the platform and build type (shared, static). 
    # (https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html)
    generate_export_header(${TARGET_NAME}
        EXPORT_FILE_NAME ${EXPORT_HEADER}
        EXPORT_MACRO_NAME ${TARGET_NAME_UPPERCASE}_API)

    if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY")
        target_compile_definitions(${TARGET_NAME} PUBLIC "${TARGET_NAME_UPPERCASE}_STATIC_DEFINE")
    endif()

    list(APPEND INPUT_PUBLIC_HEADERS ${EXPORT_HEADER})

endif()

###############################################################################
# Add the source files to the target
###############################################################################

find_include_directories(INPUT_PUBLIC_HEADERS PUBLIC_INCLUDE_DIRECTORIES)

find_include_directories(INPUT_PRIVATE_HEADERS PRIVATE_INCLUDE_DIRECTORIES)

if (INPUT_PUBLIC_HEADERS)
    target_sources(${TARGET_NAME} PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS ${PUBLIC_INCLUDE_DIRECTORIES}
        FILES ${INPUT_PUBLIC_HEADERS})
endif()

if (INPUT_PRIVATE_HEADERS)
    target_sources(${TARGET_NAME} PRIVATE
        FILE_SET private_headers
        TYPE HEADERS
        BASE_DIRS ${PRIVATE_INCLUDE_DIRECTORIES}
        FILES ${INPUT_PRIVATE_HEADERS})
endif()

if (INPUT_SOURCES)
    target_sources(${TARGET_NAME} PRIVATE
        ${INPUT_SOURCES})
endif()

###############################################################################
# Set target properties
###############################################################################

# CMake requires the language standard to be specified as compile feature
# when a target provides C++20 modules and the target will be installed 
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)

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
# Link the target
###############################################################################

# (https://cmake.org/cmake/help/latest/command/target_link_libraries.html)
target_link_libraries(${TARGET_NAME}
    PRIVATE
        ${INPUT_PRIVATE_DEPENDENCIES}
    PUBLIC
        ${INPUT_PUBLIC_DEPENDENCIES})

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