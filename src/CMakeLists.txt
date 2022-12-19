add_library(${PROJECT_NAME}Engine INTERFACE)

# Get all folders in this directory
file(GLOB ENGINE_DIRECTORIES LIST_DIRECTORIES true "${CMAKE_CURRENT_LIST_DIR}/engine/*")
foreach (ENGINE_DIRECTORY ${ENGINE_DIRECTORIES})
    if (NOT IS_DIRECTORY ${ENGINE_DIRECTORY})
        continue()
    endif ()

    # Get the name of the folder
    get_filename_component(COMPONENT_NAME ${ENGINE_DIRECTORY} NAME)

    # Capitalize first letter
    # https://stackoverflow.com/questions/11845823/converting-part-of-a-string-to-upper-case-in-cmake
    string(SUBSTRING ${COMPONENT_NAME} 0 1 COMPONENT_NAME_SUB)
    string(TOUPPER ${COMPONENT_NAME_SUB} COMPONENT_NAME_SUB)
    string(REGEX REPLACE "^.(.*)" "${COMPONENT_NAME_SUB}\\1" COMPONENT_NAME "${COMPONENT_NAME}")

    # Find all source files in the folder
    file(GLOB_RECURSE SOURCES ${ENGINE_DIRECTORY}/*.cpp ${ENGINE_DIRECTORY}/*.hpp ${ENGINE_DIRECTORY}/*.h)
    file(GLOB COMMON_HEADERS ${CMAKE_CURRENT_LIST_DIR}/engine/*.hpp ${CMAKE_CURRENT_LIST_DIR}/engine/*.h)

    # Create a library using the folder as its name
    set(LIBRARY_NAME "Engine${COMPONENT_NAME}")
    add_library(${LIBRARY_NAME} SHARED ${SOURCES} ${COMMON_HEADERS})

    # Common compile definitions
    target_compile_definitions(${LIBRARY_NAME}
            PUBLIC
            GAME_VERSION=${VERSION_STRING}
            GLM_FORCE_LEFT_HANDED=1
            MSGPACK_USE_DEFINE_MAP=1

            PRIVATE
            ENGINE_EXPORTS=1
            "ENGINE_SOURCE_PATH=${CMAKE_CURRENT_LIST_DIR}/Engine"
            _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
            _CRT_SECURE_NO_WARNINGS=1)

    # Engine public dependencies
    target_link_libraries(${LIBRARY_NAME} PUBLIC
            glm::glm
            fmt::fmt
            Cgltf
            msgpackc msgpackc-cxx
            RocksDB::rocksdb
            glad::glad
            asio
            asio::asio
            Vulkan::Vulkan
            utf8cpp)

    # Engine private dependencies
    target_link_libraries(${LIBRARY_NAME} PRIVATE
            nuklear
            glfw
            unofficial::vulkan-memory-allocator::vulkan-memory-allocator
            HLSL SPIRV glslang OGLCompiler spirv-cross-glsl
            Png
            Stb
            freetype
            RocksDB::rocksdb
            OpenSSL::SSL OpenSSL::Crypto
            OpenAL::OpenAL
            yaml-cpp
            lz4::lz4
            Python3::Python
            pybind11::lto pybind11::embed pybind11::module)

    # Platform specific dependencies
    if (WIN32)
        target_link_libraries(${LIBRARY_NAME} PRIVATE rpcrt4.lib)
        target_compile_definitions(${LIBRARY_NAME} PUBLIC _WIN32_WINNT=0x0501)
    elseif (UNIX AND NOT APPLE)
        target_link_libraries(${LIBRARY_NAME} PRIVATE ZLIB::ZLIB LibLZMA::LibLZMA unofficial::UUID::uuid uring)
    elseif (APPLE)
        target_link_libraries(${LIBRARY_NAME} PRIVATE ZLIB::ZLIB LibLZMA::LibLZMA unofficial::UUID::uuid Iconv::Iconv)
    endif ()

    # Header lookup paths
    get_filename_component(LIBRARY_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
    set_target_properties(${LIBRARY_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIBRARY_INCLUDE_DIRS})

    # Set C++17
    set_target_properties(${LIBRARY_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

    # Where the compiled output goes
    set_target_properties(${LIBRARY_NAME}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
            )

    # Add this component into the common engine library
    target_link_libraries(${PROJECT_NAME}Engine INTERFACE ${LIBRARY_NAME})
endforeach ()

# Install extras
if (WIN32)
    install(DIRECTORY "${CMAKE_BINARY_DIR}/src/TemporaryEscape/${CMAKE_BUILD_TYPE}/"
            DESTINATION "."
            FILES_MATCHING PATTERN "*.dll" PATTERN "CMakeFiles" EXCLUDE
            )
endif ()

# The main target
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "TemporaryEscape")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

if (TEMPORARY_ESCAPE_LLVM_SYMBOLIZER_PATH AND UNIX AND NOT APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(${PROJECT_NAME} PRIVATE asan)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC CLI11::CLI11 ${PROJECT_NAME}Engine)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
        )
