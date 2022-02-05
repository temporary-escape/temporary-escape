project(TemporaryEscapeFunctionalTests VERSION "${PROJECT_VERSION}")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

if(MSVC)
    add_compile_options(/wd4251)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

if(TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
    message("Enabling address sanitizer for ${PROJECT_NAME}")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(${PROJECT_NAME} PRIVATE asan)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Catch2::Catch2 TemporaryEscape)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

if(TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
    set_property(TEST ${PROJECT_NAME} APPEND PROPERTY
            ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH}"
    )
    set_property(TEST ${PROJECT_NAME} APPEND PROPERTY
            ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../supp"
    )
endif()
