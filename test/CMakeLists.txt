# Find all source files in the folder
file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

add_executable(${PROJECT_NAME}UnitTests ${SOURCES})
target_compile_definitions(${PROJECT_NAME}UnitTests PRIVATE ROOT_DIR= "${ROOT_DIR}" ENGINE_TESTS=1)

target_link_libraries(${PROJECT_NAME}UnitTests
        PUBLIC Catch2::Catch2 Catch2::Catch2WithMain EngineClient EngineServer)

set_target_properties(${PROJECT_NAME}UnitTests
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src")

add_test(NAME ${PROJECT_NAME}UnitTests COMMAND ${PROJECT_NAME}UnitTests)

if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
    set_property(TEST ${PROJECT_NAME}UnitTests APPEND PROPERTY
            ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH}")
    set_property(TEST ${PROJECT_NAME}UnitTests APPEND PROPERTY
            ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/supp:fast_unwind_on_malloc=0")
endif ()
