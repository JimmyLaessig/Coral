add_subdirectory(Coral)
add_subdirectory(Util)
add_subdirectory(Resources)

if(CORAL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

