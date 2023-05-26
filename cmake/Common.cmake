add_library(${PROJECT_NAME}Common INTERFACE)
add_library(${PROJECT_NAME}Common::${PROJECT_NAME}Common ALIAS ${PROJECT_NAME}Common)

set_target_properties(${PROJECT_NAME}Common PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

target_compile_definitions(${PROJECT_NAME}Common
        INTERFACE
        GAME_VERSION="${VERSION_STRING}"
        GLM_FORCE_LEFT_HANDED=1
        MSGPACK_USE_DEFINE_MAP=1
        _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING=1
        _CRT_SECURE_NO_WARNINGS=1
        "SOURCE_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

if (UNIX)
    target_compile_options(${PROJECT_NAME}Common INTERFACE -ftemplate-backtrace-limit=0)
elseif (MSVC)
    target_compile_options(${PROJECT_NAME}Common INTERFACE /EHsc /wd4251)
    target_link_libraries(${PROJECT_NAME}Common INTERFACE rpcrt4.lib)
    target_compile_definitions(${PROJECT_NAME}Common INTERFACE _WIN32_WINNT=0x0501)
endif ()
