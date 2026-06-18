
set(TARGET_NAME CoralResources)

add_library(${TARGET_NAME} INTERFACE)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include)

target_embed_files(${TARGET_NAME} INTERFACE
    FILE_SET
        public_headers
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/uvtest.png"
    OUTPUT_DIR
        "${GENERATED_INCLUDE_DIR}/Coral/Resources"
    BASE_DIRS 
        "${GENERATED_INCLUDE_DIR}"
    NAMESPACE
        Coral::Resources)

set(HEADERS
    ${INCLUDE_DIR}/Coral/Resources/Cube.hpp
    ${GENERATED_INCLUDE_DIR}/Coral/Resources/uvtest.hpp
)

target_sources(${TARGET_NAME} INTERFACE
    FILE_SET public_headers
    TYPE HEADERS
    BASE_DIRS
        ${INCLUDE_DIR}
        ${GENERATED_INCLUDE_DIR}
    FILES 
        ${HEADERS}
)

coral_configure_target(${TARGET_NAME})
