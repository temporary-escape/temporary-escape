if (UNIX)
    find_package(ZLIB REQUIRED)
    find_package(LibLZMA REQUIRED)
    if (APPLE)
        find_package(Iconv)
        find_package(Gettext REQUIRED)
    else ()
        find_package(unofficial-libuuid CONFIG REQUIRED)
    endif ()
endif ()

find_package(utf8cpp CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(unofficial-nuklear CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(Png REQUIRED)
find_package(Cgltf REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(msgpack CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(Uring REQUIRED)
find_package(RocksDB CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(LuaUnofficial REQUIRED)
find_package(sol2 CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(SimplexNoise REQUIRED)
find_package(Voronoi REQUIRED)
