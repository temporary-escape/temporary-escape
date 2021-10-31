include(FindPackageHandleStandardArgs)

if (NOT TARGET NanoVG)
    find_package(freetype CONFIG REQUIRED)

    set(NANOVG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/nanovg/src)
    mark_as_advanced(FORCE NANOVG_INCLUDE_DIR)
    set(NANOVG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libs/nanovg/src/nanovg.c)
    add_library(NanoVG STATIC ${NANOVG_SOURCES})
    target_compile_definitions(NanoVG PRIVATE FONS_USE_FREETYPE=1)
    target_link_libraries(NanoVG PRIVATE freetype)
    set_property(TARGET NanoVG PROPERTY POSITION_INDEPENDENT_CODE ON)
    set_target_properties(NanoVG PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NANOVG_INCLUDE_DIR})
endif ()

find_package_handle_standard_args(NanoVG DEFAULT_MSG NANOVG_INCLUDE_DIR)
