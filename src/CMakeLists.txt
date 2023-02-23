include(CMakeParseArguments)

macro(add_engine_component)
    set(_OPTIONS_ARGS)
    set(_ONE_VALUE_ARGS DIR NAME)
    set(_MULTI_VALUE_ARGS PUBLIC_LIBS PRIVATE_LIBS)

    cmake_parse_arguments(_ADD_ENGINE_COMPONENT "${_OPTIONS_ARGS}" "${_ONE_VALUE_ARGS}" "${_MULTI_VALUE_ARGS}" ${ARGN})

    file(GLOB_RECURSE SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/engine/${_ADD_ENGINE_COMPONENT_DIR}/*.cpp
            ${CMAKE_CURRENT_LIST_DIR}/engine/${_ADD_ENGINE_COMPONENT_DIR}/*.c
            ${CMAKE_CURRENT_LIST_DIR}/engine/${_ADD_ENGINE_COMPONENT_DIR}/*.hpp
            ${CMAKE_CURRENT_LIST_DIR}/engine/${_ADD_ENGINE_COMPONENT_DIR}/*.h)
    file(GLOB COMMON_HEADERS
            ${CMAKE_CURRENT_LIST_DIR}/engine/*.hpp
            ${CMAKE_CURRENT_LIST_DIR}/engine/*.h)

    # Define the library
    add_library(${_ADD_ENGINE_COMPONENT_NAME} SHARED ${SOURCES} ${COMMON_HEADERS})
    set_target_properties(${_ADD_ENGINE_COMPONENT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

    # Inherit the common options
    target_link_libraries(${_ADD_ENGINE_COMPONENT_NAME} PUBLIC ${PROJECT_NAME}Common::${PROJECT_NAME}Common)

    # Common compile definitions
    target_compile_definitions(${_ADD_ENGINE_COMPONENT_NAME}
            PRIVATE
            ENGINE_EXPORTS=1
            "ENGINE_SOURCE_PATH=${CMAKE_CURRENT_LIST_DIR}/Engine")

    # Header lookup paths
    set_target_properties(${_ADD_ENGINE_COMPONENT_NAME}
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}")

    # Where the compiled output goes
    set_target_properties(${_ADD_ENGINE_COMPONENT_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

    # Platform specific dependencies
    if (UNIX AND NOT APPLE)
        target_link_libraries(${_ADD_ENGINE_COMPONENT_NAME}
                INTERFACE
                ZLIB::ZLIB
                LibLZMA::LibLZMA
                unofficial::UUID::uuid
                uring)
    elseif (APPLE)
        target_link_libraries(${_ADD_ENGINE_COMPONENT_NAME}
                INTERFACE
                ZLIB::ZLIB
                LibLZMA::LibLZMA
                unofficial::UUID::uuid
                Iconv::Iconv)
    endif ()

    # Component specific libraries
    target_link_libraries(${_ADD_ENGINE_COMPONENT_NAME} PUBLIC ${_ADD_ENGINE_COMPONENT_PUBLIC_LIBS})
    target_link_libraries(${_ADD_ENGINE_COMPONENT_NAME} PRIVATE ${_ADD_ENGINE_COMPONENT_PRIVATE_LIBS})
endmacro()

add_engine_component(
        NAME EngineUtils
        DIR utils
        PUBLIC_LIBS fmt::fmt asio asio::asio utf8cpp
        PRIVATE_LIBS Png yaml-cpp OpenSSL::SSL OpenSSL::Crypto)

add_engine_component(
        NAME EngineMath
        DIR math
        PUBLIC_LIBS EngineUtils glm::glm)

add_engine_component(
        NAME EngineAudio
        DIR audio
        PUBLIC_LIBS EngineUtils
        PRIVATE_LIBS OpenAL::OpenAL)

add_engine_component(
        NAME EngineVulkan
        DIR vulkan
        PUBLIC_LIBS Vulkan::Vulkan EngineUtils
        PRIVATE_LIBS
        glfw
        glslang::glslang
        glslang::SPIRV
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        spirv-cross-glsl)

add_engine_component(
        NAME EngineFont
        DIR font
        PUBLIC_LIBS EngineVulkan EngineUtils
        PRIVATE_LIBS freetype)

add_engine_component(
        NAME EngineAssets
        DIR assets
        PUBLIC_LIBS EngineVulkan EngineMath EngineUtils)

add_engine_component(
        NAME EngineScene
        DIR scene
        PUBLIC_LIBS EnTT::EnTT EngineAssets EngineFont)

add_engine_component(
        NAME EngineGraphics
        DIR graphics
        PUBLIC_LIBS EngineScene
        PRIVATE_LIBS nuklear)

add_engine_component(
        NAME EngineNetwork
        DIR network
        PUBLIC_LIBS EngineMath EngineUtils
        PRIVATE_LIBS OpenSSL::SSL OpenSSL::Crypto lz4::lz4)

add_engine_component(
        NAME EngineDatabase
        DIR database
        PUBLIC_LIBS EngineMath EngineUtils
        PRIVATE_LIBS RocksDB::rocksdb)

add_engine_component(
        NAME EngineServices
        DIR services
        PUBLIC_LIBS EngineAssets EngineDatabase EngineNetwork OpenSSL::SSL OpenSSL::Crypto)

add_engine_component(
        NAME EngineServer
        DIR server
        PUBLIC_LIBS EngineServices EngineScene EngineMath
        PythonUnofficial::PythonUnofficial Pybind11Unofficial::Pybind11Unofficial)

add_engine_component(
        NAME EngineClient
        DIR client
        PUBLIC_LIBS EngineGraphics EngineServer EngineAudio)

# The main target
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "TemporaryEscape")

target_link_libraries(${PROJECT_NAME}
        PUBLIC CLI11::CLI11 EngineClient EngineServer)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
