---
weight: 1
title: "Libraries"
---

# Libraries

List of libraries used in this game.

* [wren](https://wren.io/) - Wren, an embeddable scripting language. Used for scripting purposes and modding.
* [wrenbind17](https://github.com/matusnovak/wrenbind17) - A header only library for binding C++17 to Wren language.
* [nuklear](https://github.com/Immediate-Mode-UI/Nuklear) - A single-header ANSI C immediate mode cross-platform GUI library.
* [zlib](https://zlib.net/) - Needed by libpng.
* [glfw3](https://www.glfw.org/) - Window and OpenGL context management.
* [glm](https://github.com/g-truc/glm) - OpenGL header only C++ mathematics library.
* [glad](https://glad.dav1d.de/) - OpenGL extensions loader header only library.
* [fmt](https://github.com/fmtlib/fmt) - Formatting library providing a fast and safe alternative to C stdio and C++ iostreams.
* [libxml2](http://xmlsoft.org/) - XML C parser and toolkit developed for the Gnome project.
* [nanovg](https://github.com/memononen/nanovg) - NanoVG is small antialiased vector graphics rendering library for OpenGL. Used for rendering simple shapes, text, and the GUI.
* [libpng](http://www.libpng.org/pub/png/libpng.html) - Official PNG reference library. Used for loading png assets.
* [cgltf](https://github.com/jkuhlmann/cgltf) - Single-file/stb-style C glTF loader and writer. Used for loading gltf assets.
* [msgpack-c](https://msgpack.org/index.html) - MessagePack is an efficient binary serialization format. Used for data structure serialization and deserialization in the networking stack and the database.
* [asio](https://think-async.com/Asio/) - A cross-platform C++ library for network and low-level I/O programming. Used for low level networking and background workers.
* [stb](https://github.com/nothings/stb) - Single-file public domain libraries. Only the `stb_rect_pack.h` part is used for packing images into image atlases.
* [catch2](https://github.com/catchorg/Catch2) - A modern, C++-native, test framework for unit-tests. Used for unit tests and functional tests.
* [spdlog](https://github.com/gabime/spdlog) - Fast C++ logging library. 
* [cli11](https://github.com/CLIUtils/CLI11) - Command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface. 
* [freetype](https://freetype.org/) - True type font loader and glyph renderer. Used by nanovg to load and render fonts.
* [rocksdb](http://rocksdb.org/) - An embeddable persistent key-value store for fast storage. Used as a data store (with msgpack) to persistently store world data.
* [openssl](https://www.openssl.org/) - Toolkit for general-purpose cryptography and secure communication. Used on top of a custom networking stack with asio.
* [libuuid](https://linux.die.net/man/3/libuuid) (Linux & Mac OSX only) - Used for generating UUID strings.
* [liblzma](https://github.com/xz-mirror/xz) - Free general-purpose data compression software with high compression ratio.
