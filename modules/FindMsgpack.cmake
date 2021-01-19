include(FindPackageHandleStandardArgs)


if(NOT TARGET Msgpack)
    find_path(MSGPACK_INCLUDE_DIR NAMES msgpack/pack.hpp)
    mark_as_advanced(FORCE MSGPACK_INCLUDE_DIR)
    add_library(Msgpack INTERFACE IMPORTED)
    set_target_properties(Msgpack PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${MSGPACK_INCLUDE_DIR})
endif()

find_package_handle_standard_args(Msgpack DEFAULT_MSG MSGPACK_INCLUDE_DIR)
