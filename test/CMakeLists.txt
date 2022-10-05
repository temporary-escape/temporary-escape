get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

# Get all folders in this directory
set(ENGINE_TEST_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/UnitTests" "${CMAKE_CURRENT_LIST_DIR}/FunctionalTests")

foreach (ENGINE_TEST_DIRECTORY ${ENGINE_TEST_DIRECTORIES})
    # Get the name of the folder
    get_filename_component(COMPONENT_NAME ${ENGINE_TEST_DIRECTORY} NAME)

    # Find all source files in the folder
    file(GLOB_RECURSE SOURCES ${ENGINE_TEST_DIRECTORY}/*.cpp ${ENGINE_TEST_DIRECTORY}/*.hpp ${ENGINE_TEST_DIRECTORY}/*.h)
    file(GLOB COMMON_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Common/*.hpp ${CMAKE_CURRENT_LIST_DIR}/Common/*.h)

    add_executable(${COMPONENT_NAME} ${SOURCES})
    set_target_properties(${COMPONENT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
    target_compile_definitions(${COMPONENT_NAME} PRIVATE ROOT_DIR="${ROOT_DIR}" ENGINE_TESTS=1)

    if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
        target_compile_options(${COMPONENT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_libraries(${COMPONENT_NAME} PRIVATE asan)
    endif ()

    target_link_libraries(${COMPONENT_NAME} PUBLIC Catch2::Catch2 Catch2::Catch2WithMain ${PROJECT_NAME}Engine)

    set_target_properties(${COMPONENT_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src")

    add_test(NAME ${COMPONENT_NAME} COMMAND ${PROJECT_NAME})

    if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
        set_property(TEST ${COMPONENT_NAME} APPEND PROPERTY
                ENVIRONMENT "ASAN_SYMBOLIZER_PATH=${TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH}")
        set_property(TEST ${COMPONENT_NAME} APPEND PROPERTY
                ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/supp:fast_unwind_on_malloc=0")
    endif ()

endforeach ()
