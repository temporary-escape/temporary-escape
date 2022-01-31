file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

if (MSVC)
    add_compile_options(/wd4251)
endif ()

add_executable(${PROJECT_NAME}UnitTests ${SOURCES})
target_link_libraries(${PROJECT_NAME}UnitTests PUBLIC Catch2::Catch2 ${PROJECT_NAME}Lib)
set_target_properties(${PROJECT_NAME}UnitTests PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

set_target_properties(${PROJECT_NAME}UnitTests
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/Scissio"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/Scissio"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src/Scissio"
        )
