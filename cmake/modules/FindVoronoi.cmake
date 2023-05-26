include(ExternalProject)

find_package(Git)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/third_party")

execute_process(
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/JCash/voronoi.git"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/third_party")

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/voronoi/src/jc_voronoi.h
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/voronoi/src/jc_voronoi_clip.h)

add_library(Voronoi INTERFACE ${SOURCES})
add_library(Voronoi::Voronoi ALIAS Voronoi)
set_target_properties(Voronoi PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/third_party/voronoi/src")
