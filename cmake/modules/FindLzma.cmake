include(FindPackageHandleStandardArgs)

if (NOT TARGET Lzma)
    find_library(LZMA_LIBRARIES NAMES lzma)
    find_path(LZMA_INCLUDE_DIR NAMES lzma.h)
    add_library(Lzma STATIC IMPORTED)
    set_target_properties(Lzma PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LZMA_INCLUDE_DIR})
    set_target_properties(Lzma PROPERTIES IMPORTED_LOCATION ${LZMA_LIBRARIES})
endif ()

find_package_handle_standard_args(Lzma DEFAULT_MSG LZMMA_LIBRARIES LZMA_INCLUDE_DIR)
