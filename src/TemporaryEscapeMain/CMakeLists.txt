project(TemporaryEscapeMain VERSION "${PROJECT_VERSION}")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/Main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "TemporaryEscape")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(${PROJECT_NAME} PRIVATE asan)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC CLI11::CLI11 TemporaryEscape)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        )

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "."
        LIBRARY DESTINATION "."
        ARCHIVE DESTINATION "."
        )
