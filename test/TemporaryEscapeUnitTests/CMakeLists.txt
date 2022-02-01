project(TemporaryEscapeUnitTests VERSION "${PROJECT_VERSION}")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

if (MSVC)
    add_compile_options(/wd4251)
endif ()

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC Catch2::Catch2 TemporaryEscape)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
