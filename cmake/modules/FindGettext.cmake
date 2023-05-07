include(FindPackageHandleStandardArgs)

if(NOT TARGET Gettext)
    find_library(GETTEXT_LIBRARIES NAMES intl)
    add_library(Gettext STATIC IMPORTED)
    set_target_properties(Gettext PROPERTIES IMPORTED_LOCATION ${GETTEXT_LIBRARIES})
endif()

find_package_handle_standard_args(Gettext DEFAULT_MSG GETTEXT_LIBRARIES)
