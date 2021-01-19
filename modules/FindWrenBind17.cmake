include(FindPackageHandleStandardArgs)

if(NOT TARGET WrenBind17)
    find_path(WRENBIND17_INCLUDE_DIR NAMES wrenbind17 PATHS ${CMAKE_CURRENT_LIST_DIR}/../libs/wrenbind17/include)
    mark_as_advanced(FORCE WRENBIND17_INCLUDE_DIR)
    add_library(WrenBind17 INTERFACE IMPORTED)
    set_target_properties(WrenBind17 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${WRENBIND17_INCLUDE_DIR})
endif()

find_package_handle_standard_args(WrenBind17 DEFAULT_MSG WRENBIND17_INCLUDE_DIR)
