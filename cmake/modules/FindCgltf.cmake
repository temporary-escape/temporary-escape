include(FindPackageHandleStandardArgs)

if(NOT TARGET Cgltf)
    find_path(CGLTF_INCLUDE_DIR NAMES cgltf.h)
    mark_as_advanced(FORCE CGLTF_INCLUDE_DIR)
    add_library(Cgltf INTERFACE IMPORTED)
    set_target_properties(Cgltf PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CGLTF_INCLUDE_DIR})
endif()

find_package_handle_standard_args(Cgltf DEFAULT_MSG CGLTF_INCLUDE_DIR)
