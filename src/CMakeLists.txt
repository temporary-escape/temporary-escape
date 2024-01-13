include(CMakeParseArguments)

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/Engine/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Engine/*.c
        ${CMAKE_CURRENT_LIST_DIR}/Engine/*.hpp
        ${CMAKE_CURRENT_LIST_DIR}/Engine/*.h)

# Define the library
add_library(${PROJECT_NAME}Engine SHARED ${SOURCES})
set_target_properties(${PROJECT_NAME}Engine PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
set_target_properties(${PROJECT_NAME}Engine PROPERTIES OUTPUT_NAME "${PROJECT_NAME}Engine")

if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH)
    target_compile_options(${PROJECT_NAME}Engine PUBLIC -fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address)
endif ()

# Inherit the common options
target_link_libraries(${PROJECT_NAME}Engine PUBLIC ${PROJECT_NAME}Common::${PROJECT_NAME}Common)

# Embedded files
get_property(SHADERS_DIR TARGET ${PROJECT_NAME}Shaders PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
get_property(DATA_DIR TARGET ${PROJECT_NAME}Data PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME}Engine
        PRIVATE
        "${SHADERS_DIR}"
        "${DATA_DIR}")

add_dependencies(${PROJECT_NAME}Engine ${PROJECT_NAME}Shaders ${PROJECT_NAME}Data)

# Common compile definitions
target_compile_definitions(${PROJECT_NAME}Engine
        PUBLIC
        SOL_ALL_SAFETIES_ON=1
        PRIVATE
        ENGINE_EXPORTS=1
        "ENGINE_SOURCE_PATH=${CMAKE_CURRENT_LIST_DIR}/Engine")

# Header lookup paths
set_target_properties(${PROJECT_NAME}Engine
        PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}")

# Where the compiled output goes
set_target_properties(${PROJECT_NAME}Engine
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Platform specific dependencies
if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}Engine
            PRIVATE
            ZLIB::ZLIB
            LibLZMA::LibLZMA
            unofficial::UUID::uuid
            uring::uring)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME}Engine
            PRIVATE
            ZLIB::ZLIB
            LibLZMA::LibLZMA
            Iconv::Iconv
            Gettext)
endif ()

# Component specific libraries
target_link_libraries(${PROJECT_NAME}Engine
        PUBLIC
        fmt::fmt
        asio
        asio::asio
        utf8cpp
        spdlog::spdlog
        glm::glm
        VulkanHeaders::VulkanHeaders
        EnTT::EnTT
        PRIVATE
        Png
        LibXml2::LibXml2
        OpenSSL::SSL
        OpenSSL::Crypto
        OpenAL::OpenAL
        glfw
        volk::volk
        volk::volk_headers
        glslang::glslang
        glslang::SPIRV
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        spirv-cross-glsl
        freetype
        SimplexNoise
        Voronoi
        nuklear
        OpenSSL::SSL
        OpenSSL::Crypto
        lz4::lz4
        RocksDB::rocksdb
        LuaUnofficial
        sol2
        AdMipMap
        KTX::ktx
        ${BULLET_LIBRARIES}
        Vorbis::vorbis
        Vorbis::vorbisfile
        MicroPather
        libmorton::libmorton)

# The main target
if (MSVC)
    add_executable(${PROJECT_NAME}Main
            WIN32
            "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/windows/appicon.rc"
            "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
else ()
    add_executable(${PROJECT_NAME}Main ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
endif ()

set_target_properties(${PROJECT_NAME}Main PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH)
    target_link_options(${PROJECT_NAME}Main PRIVATE -lasan)
endif ()

target_link_libraries(${PROJECT_NAME}Main
        PRIVATE CLI11::CLI11
        ${PROJECT_NAME}Engine
        OpenSSL::SSL
        OpenSSL::Crypto)

set_target_properties(${PROJECT_NAME}Main
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

install(TARGETS ${PROJECT_NAME}Main ${PROJECT_NAME}Engine
        LIBRARY DESTINATION "."
        RUNTIME DESTINATION ".")

install(CODE "execute_process(COMMAND $<TARGET_FILE:${PROJECT_NAME}Main> compress-assets --root \"${CMAKE_SOURCE_DIR}\")")

if (MSVC)
    # Start menu shortcut
    set_property(INSTALL
            "$<TARGET_FILE_NAME:${PROJECT_NAME}Main>"
            PROPERTY CPACK_START_MENU_SHORTCUTS "Temporary Escape")
    # Desktop shortcut
    set_property(INSTALL
            "$<TARGET_FILE_NAME:${PROJECT_NAME}Main>"
            PROPERTY CPACK_DESKTOP_SHORTCUTS "Temporary Escape")
endif ()
