project(TemporaryEscape VERSION "${PROJECT_VERSION}")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC GAME_VERSION=${VERSION_STRING})
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        glm::glm
        fmt::fmt
        Cgltf
        msgpackc msgpackc-cxx
        RocksDB::rocksdb
        glad::glad
        freetype
        Wren
        WrenBind17
        PRIVATE
        glfw
        NanoVG
        Png
        asio
        asio::asio
        Nuklear
        Stb
        RocksDB::rocksdb
        OpenSSL::SSL OpenSSL::Crypto
        OpenAL::OpenAL
        yaml-cpp
        Voronoi
        )

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE rpcrt4.lib)
elseif (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB LibLZMA::LibLZMA unofficial::UUID::uuid uring)
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB LibLZMA::LibLZMA unofficial::UUID::uuid Iconv::Iconv)
endif ()

# Library definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
        ENGINE_EXPORTS=1
        _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
        _CRT_SECURE_NO_WARNINGS=1)

# Msgpack-c library definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC
        MSGPACK_USE_DEFINE_MAP=1)
if (WIN32)
    # Asio library networking definitions
    target_compile_definitions(${PROJECT_NAME} PUBLIC
            _WIN32_WINNT=0x0501)
endif ()

get_filename_component(PROJECT_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PROJECT_INCLUDE_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
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

if (WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape/${CMAKE_BUILD_TYPE}/"
            DESTINATION "."
            FILES_MATCHING PATTERN "*.dll" PATTERN "CMakeFiles" EXCLUDE
            )
endif ()
