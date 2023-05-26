include(ExternalProject)

find_package(Git)

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/third_party")

execute_process(
        COMMAND ${GIT_EXECUTABLE} clone "https://github.com/SRombauts/SimplexNoise.git"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/third_party")

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.c
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src/*.h)

add_library(SimplexNoise STATIC ${SOURCES})
add_library(SimplexNoise::SimplexNoise ALIAS SimplexNoise)
set_target_properties(SimplexNoise PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
set_target_properties(SimplexNoise PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}/third_party/SimplexNoise/src")
