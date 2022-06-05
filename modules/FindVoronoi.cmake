include(FindPackageHandleStandardArgs)

if (NOT TARGET Voronoi)
    set(VORONOI_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/../libs/voronoi/src)
    mark_as_advanced(FORCE VORONOI_INCLUDE_DIR)
    add_library(Voronoi INTERFACE IMPORTED)
    set_target_properties(Voronoi PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${VORONOI_INCLUDE_DIR})
endif ()

find_package_handle_standard_args(Voronoi DEFAULT_MSG VORONOI_INCLUDE_DIR)
