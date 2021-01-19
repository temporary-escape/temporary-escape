include(FindPackageHandleStandardArgs)

if(NOT TARGET IL)
    find_library(IL_LIBRARIES NAMES DevIL IL)
    find_path(IL_INCLUDE_DIR NAMES IL/il.h)
    add_library(IL STATIC IMPORTED)
    set_target_properties(IL PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${IL_INCLUDE_DIR})
    set_target_properties(IL PROPERTIES IMPORTED_LOCATION ${IL_LIBRARIES})
endif()

find_package_handle_standard_args(IL DEFAULT_MSG IL_LIBRARIES IL_INCLUDE_DIR)
