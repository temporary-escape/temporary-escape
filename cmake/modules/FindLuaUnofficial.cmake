include(FindPackageHandleStandardArgs)

if (NOT TARGET LuaUnofficial)
    find_library(LUA_LIBRARIES NAMES lua)
    find_path(LUA_INCLUDE_DIR NAMES lua.h)
    add_library(LuaUnofficial STATIC IMPORTED)
    set_target_properties(LuaUnofficial PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR})
    set_target_properties(LuaUnofficial PROPERTIES IMPORTED_LOCATION ${LUA_LIBRARIES})
endif ()

find_package_handle_standard_args(LuaUnofficial DEFAULT_MSG LUA_LIBRARIES LUA_INCLUDE_DIR)
