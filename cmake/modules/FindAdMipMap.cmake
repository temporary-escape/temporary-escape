include(ExternalProject)

find_package(Git)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/third_party")

execute_process(
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/Vavassor/ad_mipmap.git"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/third_party")

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.c
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.h)

add_library(AdMipMap INTERFACE ${SOURCES})
add_library(AdMipMap::AdMipMap ALIAS AdMipMap)
set_target_properties(AdMipMap PROPERTIES C_STANDARD 11)
set_target_properties(AdMipMap PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/third_party/ad_mipmap")
