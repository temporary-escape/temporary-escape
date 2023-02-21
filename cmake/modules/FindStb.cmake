include(FindPackageHandleStandardArgs)

if (NOT TARGET Stb)
    find_path(STB_INCLUDE_DIR NAMES stb_c_lexer.h)
    mark_as_advanced(FORCE STB_INCLUDE_DIR)
    add_library(Stb INTERFACE IMPORTED)
    set_target_properties(Stb PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${STB_INCLUDE_DIR})
endif ()

find_package_handle_standard_args(Stb DEFAULT_MSG STB_INCLUDE_DIR)
