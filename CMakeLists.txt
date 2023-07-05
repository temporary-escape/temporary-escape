cmake_minimum_required(VERSION 3.21)
project(TemporaryEscape)

set(CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/GitVersion.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Options.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Common.cmake)

if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/AddressSanitizer.cmake)
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Dependencies.cmake)

if (UNIX)
    if (APPLE)
        set(CMAKE_INSTALL_RPATH "@executable_path")
    else ()
        set(CMAKE_INSTALL_RPATH "$ORIGIN")
    endif ()
endif ()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src)

# Unit tests
if (TEMPORARY_ESCAPE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/test)
endif ()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/InstallDirs.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Packaging.cmake)
