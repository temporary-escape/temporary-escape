include(CMakeParseArguments)

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/engine/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/engine/*.c
        ${CMAKE_CURRENT_LIST_DIR}/engine/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/engine/*.h)

# Define the library
add_library(${PROJECT_NAME}Library SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME}Library PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
set_target_properties(${PROJECT_NAME}Library PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

# Inherit the common options
target_link_libraries(${PROJECT_NAME}Library PUBLIC ${PROJECT_NAME}Common::${PROJECT_NAME}Common)

# Common compile definitions
target_compile_definitions(${PROJECT_NAME}Library
        PRIVATE
        ENGINE_EXPORTS=1
        "ENGINE_SOURCE_PATH=${CMAKE_CURRENT_LIST_DIR}/Engine")

# Header lookup paths
set_target_properties(${PROJECT_NAME}Library
        PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}")

# Where the compiled output goes
set_target_properties(${PROJECT_NAME}Library
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Platform specific dependencies
if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}Library
            PRIVATE
            ZLIB::ZLIB
            LibLZMA::LibLZMA
            unofficial::UUID::uuid
            uring::uring)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME}Library
            PRIVATE
            ZLIB::ZLIB
            LibLZMA::LibLZMA
            unofficial::UUID::uuid
            Iconv::Iconv)
endif ()

# Component specific libraries
target_link_libraries(${PROJECT_NAME}Library
        PUBLIC
        fmt::fmt
        asio
        asio::asio
        utf8cpp
        spdlog::spdlog
        glm::glm
        Vulkan::Vulkan
        EnTT::EnTT)

target_link_libraries(${PROJECT_NAME}Library
        PRIVATE
        Png
        yaml-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        OpenAL::OpenAL
        glfw
        glslang::glslang
        glslang::SPIRV
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        spirv-cross-glsl
        freetype
        nuklear
        OpenSSL::SSL
        OpenSSL::Crypto
        lz4::lz4
        RocksDB::rocksdb
        PythonUnofficial::PythonUnofficial
        Pybind11Unofficial::Pybind11Unofficial)

# The main target
add_executable(${PROJECT_NAME}Main ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set_target_properties(${PROJECT_NAME}Main PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

target_link_libraries(${PROJECT_NAME}Main
        PRIVATE CLI11::CLI11
        ${PROJECT_NAME}Library
        OpenSSL::SSL
        OpenSSL::Crypto)

set_target_properties(${PROJECT_NAME}Main
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_custom_command(
        TARGET ${PROJECT_NAME}Main
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        -E copy_directory ${CPYTHON_STDLIB_DIR} ${CMAKE_BINARY_DIR}/python)

install(TARGETS ${PROJECT_NAME}Main ${PROJECT_NAME}Library
        LIBRARY DESTINATION "."
        RUNTIME DESTINATION ".")
