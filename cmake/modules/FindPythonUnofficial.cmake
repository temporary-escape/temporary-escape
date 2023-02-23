include(FindPackageHandleStandardArgs)


find_library(PYTHON3_LIBRARY_RELEASE
        NAMES python310)
if (MSVC)
    find_library(PYTHON3_LIBRARY_DEBUG
            NAMES python310_d)
endif ()
find_path(PYTHON3_INCLUDE_DIR
        NAMES python3.10/Python.h)

if (PYTHON3_INCLUDE_DIR)
    set(PYTHON3_INCLUDE_DIR "${PYTHON3_INCLUDE_DIR}/python3.10")
endif ()

if (PYTHON3_LIBRARY_RELEASE AND MSVC)
    get_filename_component(PYTHON3_PARENT_DIR "${PYTHON3_LIBRARY_RELEASE}" DIRECTORY)
    set(PYTHON3_DLL_RELEASE "${PYTHON3_PARENT_DIR}/../bin/python310.dll")
endif ()

if (PYTHON3_LIBRARY_DEBUG AND MSVC)
    get_filename_component(PYTHON3_PARENT_DIR "${PYTHON3_LIBRARY_DEBUG}" DIRECTORY)
    set(PYTHON3_DLL_DEBUG "${PYTHON3_PARENT_DIR}/../bin/python310_d.dll")
endif ()

if (MSVC)
    find_package_handle_standard_args(PythonUnofficial REQUIRED_VARS
            PYTHON3_LIBRARY_RELEASE PYTHON3_LIBRARY_DEBUG PYTHON3_DLL_RELEASE PYTHON3_DLL_DEBUG PYTHON3_INCLUDE_DIR)
else ()
    find_package_handle_standard_args(PythonUnofficial REQUIRED_VARS
            PYTHON3_LIBRARY_RELEASE PYTHON3_INCLUDE_DIR)
endif ()

if (PYTHONUNOFFICIAL_FOUND)
    mark_as_advanced(PYTHON3_LIBRARY_RELEASE)
    mark_as_advanced(PYTHON3_LIBRARY_DEBUG)
    mark_as_advanced(PYTHON3_INCLUDE_DIR)
    if (MSVC)
        mark_as_advanced(PYTHON3_DLL_RELEASE)
        mark_as_advanced(PYTHON3_DLL_DEBUG)
    endif ()
endif ()

if (PYTHONUNOFFICIAL_FOUND AND NOT TARGET PythonUnofficial)
    add_library(PythonUnofficial SHARED IMPORTED GLOBAL)
    add_library(PythonUnofficial::PythonUnofficial ALIAS PythonUnofficial)
    target_include_directories(PythonUnofficial INTERFACE ${PYTHON3_INCLUDE_DIR})
    if (MSVC)
        set_property(TARGET PythonUnofficial PROPERTY IMPORTED_LOCATION_RELEASE ${PYTHON3_DLL_RELEASE})
        set_property(TARGET PythonUnofficial PROPERTY IMPORTED_IMPLIB_RELEASE ${PYTHON3_LIBRARY_RELEASE})
        set_property(TARGET PythonUnofficial PROPERTY IMPORTED_LOCATION_DEBUG ${PYTHON3_DLL_DEBUG})
        set_property(TARGET PythonUnofficial PROPERTY IMPORTED_IMPLIB_DEBUG ${PYTHON3_LIBRARY_DEBUG})
    else ()
        set_property(TARGET PythonUnofficial PROPERTY IMPORTED_LOCATION ${PYTHON3_LIBRARY_RELEASE})
    endif ()

    add_custom_target(PythonUnofficialPostBuild POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:PythonUnofficial> ${CMAKE_BINARY_DIR})
endif ()
