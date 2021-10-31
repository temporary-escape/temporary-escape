file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_LIST_DIR}/Main.cpp)

if (MSVC)
    add_compile_options(/wd4251)
endif ()

add_library(${PROJECT_NAME}Lib SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME}Lib
        PUBLIC
        glm
        fmt::fmt
        Cgltf
        Msgpack
        RocksDB::rocksdb
        glad::glad
        freetype
        PRIVATE
        glfw
        Xml2
        NanoVG
        Png
        asio
        asio::asio
        Nuklear
        Stb
        Wren
        WrenBind17
        RocksDB::rocksdb
        )

if (NOT MSVC)
    target_link_libraries(${PROJECT_NAME}Lib PRIVATE ZLIB::ZLIB liblzma::liblzma)
endif ()

target_compile_definitions(${PROJECT_NAME}Lib PRIVATE
        SCISSIO_EXPORTS=1
        _WIN32_WINNT=0x0501
        _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
        _CRT_SECURE_NO_WARNINGS=1
        )
set_target_properties(${PROJECT_NAME}Lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(${PROJECT_NAME}Lib PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/Main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC cxxopts::cxxopts ${PROJECT_NAME}Lib)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)
