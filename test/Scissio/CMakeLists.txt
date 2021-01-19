file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

add_compile_options(/wd4251)

add_executable(${PROJECT_NAME}Tests ${SOURCES})
target_link_libraries(${PROJECT_NAME}Tests PUBLIC Catch2::Catch2 ${PROJECT_NAME}Lib)
set_target_properties(${PROJECT_NAME}Tests PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

set_target_properties(${PROJECT_NAME}Tests
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/Scissio"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/Scissio"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/Scissio"
)
