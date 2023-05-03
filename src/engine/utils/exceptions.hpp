#pragma once

#include "log.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace Engine {

ENGINE_API void backtrace(Logger& logger, const std::exception& e);
} // namespace Engine

constexpr const char* pathstem(const char* path) {
    const auto* file = path;
    while (*path) {
#ifdef _WIN32
        if (*path++ == '\\') {
#else
        if (*path++ == '/') {
#endif
            file = path;
        }
    }
    return file;
}

#define __FILENAME__ pathstem(__FILE__)

#define EXCEPTION_VA_ARGS(...) , ##__VA_ARGS__

#define EXCEPTION(MSG, ...)                                                                                            \
    throw std::runtime_error(                                                                                          \
        fmt::format(std::string("Exception at {}:{} ") + MSG, __FILENAME__, __LINE__ EXCEPTION_VA_ARGS(__VA_ARGS__)))

#define EXCEPTION_NESTED(MSG, ...)                                                                                     \
    std::throw_with_nested(std::runtime_error(                                                                         \
        fmt::format(std::string("Exception at {}:{} ") + MSG, __FILENAME__, __LINE__ EXCEPTION_VA_ARGS(__VA_ARGS__))))

#define BACKTRACE(EXP, MESSAGE, ...)                                                                                   \
    logger.error("Error at {} {}", WHERE, fmt::format(MESSAGE EXCEPTION_VA_ARGS(__VA_ARGS__)));                        \
    Engine::backtrace(logger, EXP);

#define WHERE (__FILENAME__ + std::string(":") + std::to_string(__LINE__))
