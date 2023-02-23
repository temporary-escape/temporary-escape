include(FindPackageHandleStandardArgs)

find_path(PYBIND11_INCLUDE_DIR
        NAMES pybind11/pybind11.h)

find_package_handle_standard_args(Pybind11Unofficial REQUIRED_VARS PYBIND11_INCLUDE_DIR)

if (PYBIND11UNOFFICIAL_FOUND)
    mark_as_advanced(PYBIND11_INCLUDE_DIR)
endif ()

if (PYBIND11UNOFFICIAL_FOUND AND NOT TARGET Pybind11Unofficial)
    add_library(Pybind11Unofficial INTERFACE IMPORTED GLOBAL)
    add_library(Pybind11Unofficial::Pybind11Unofficial ALIAS Pybind11Unofficial)
    target_include_directories(Pybind11Unofficial INTERFACE ${PYBIND11_INCLUDE_DIR})
endif ()
