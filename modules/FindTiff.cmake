include(FindPackageHandleStandardArgs)

if(NOT TARGET Tiff)
    find_library(TIFF_LIBRARIES NAMES tiff)
    find_path(TIFF_INCLUDE_DIR NAMES tiff.h)
    add_library(Tiff STATIC IMPORTED)
    set_target_properties(Tiff PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${TIFF_INCLUDE_DIR})
    set_target_properties(Tiff PROPERTIES IMPORTED_LOCATION ${TIFF_LIBRARIES})
endif()

find_package_handle_standard_args(Tiff DEFAULT_MSG TIFF_LIBRARIES TIFF_INCLUDE_DIR)
