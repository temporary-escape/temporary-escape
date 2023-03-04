---
weight: 33
title: "Building"
---

# Building

{{< hint info >}}
Manually building is boring. It is recommended that you use one of the IDEs as described [here]({{< ref "setup/_index.md" >}}).
{{< /hint >}}

## 1. Prerequisites

Make sure that you have installed [dependencies]({{< ref "dependencies.md" >}})

## 2. Cloning

Clone this project including its git submodules.

```bash
git clone https://github.com/matusnovak/temporary-escape.git
git submodule update --init
```

## 3. Configure project

{{< hint warning >}}
**Last argument is a dot**  
Don't forget to specify which folder the CMake should run the configuration from. The last argument must be `.`, the current folder.
{{< /hint >}}

### Linux

Find where Clang++ is located on your system. This is not needed if using Visual Studio.

```bash
which clang++
# Should output: /usr/bin/clang++
```

Configure the project by specifying vcpkg toolchain file and Clang compiler. This will use vcpkg to download and install all required C++ libraries. This may take few minutes, but needs to be done only once.

```bash
cd temporary-escape
mkdir build
CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake \
    -B ./build \
    -G "Ninja" \
    -DTEMPORARY_ESCAPE_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake .
```

### Windows

```bat
cd temporary-escape
mkdir build
cmake \
    -B ./build \
    -G "Visual Studio 19 2019" -A x64 -T ClangCL \
    -DTEMPORARY_ESCAPE_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=C:\tools\vcpkg\scripts\buildsystems\vcpkg.cmake .
```

### Mac OSX

```bash
cd temporary-escape
mkdir build
CC=/usr/bin/clang CXX=/usr/bin/clang++ cmake \
    -B ./build \
    -G "Ninja" \
    -DTEMPORARY_ESCAPE_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake .
```

## Build

```bash
cd temporary-escape
cmake --build ./build --target TemporaryEscapeMain
```

Binary file will be generated in `${CMAKE_BINARY_DIR}/src/TemporaryEscape`.

## 4. Running

### Linux

You need to provide `--root` because by default the executable `TemporaryEscape` expects that it is 1 folder down relative to where `assets` and `shaders` folders are located.

```bash
cd temporary-escape
./build/src/TemporaryEscape/TemporaryEscape --root $(pwd)
```

### Windows

```bat
cd temporary-escape
.\build\src\TemporaryEscape\Debug\TemporaryEscape.exe --root $(pwd)
```

### Mac OSX

TBA
