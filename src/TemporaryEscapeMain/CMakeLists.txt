project(TemporaryEscapeMain)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/Main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC cxxopts::cxxopts TemporaryEscape)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "TemporaryEscape")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape"
)
