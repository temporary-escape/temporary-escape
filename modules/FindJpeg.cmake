include(FindPackageHandleStandardArgs)

if(NOT TARGET Jpeg)
    find_library(JPEG_LIBRARIES NAMES jpeg)
    find_path(JPEG_INCLUDE_DIR NAMES jpeglib.h)
    add_library(Jpeg STATIC IMPORTED)
    set_target_properties(Jpeg PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${JPEG_INCLUDE_DIR})
    set_target_properties(Jpeg PROPERTIES IMPORTED_LOCATION ${JPEG_LIBRARIES})
endif()

find_package_handle_standard_args(Jpeg DEFAULT_MSG JPEG_LIBRARIES JPEG_INCLUDE_DIR)
