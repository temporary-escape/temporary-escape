#pragma once

#include "Log.hpp"
#include <filesystem>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace Engine {

ENGINE_API void backtrace(Logger& logger, const std::exception& e);
} // namespace Engine

#define EXCEPTION_VA_ARGS(...) , ##__VA_ARGS__

#define EXCEPTION(MSG, ...)                                                                                            \
    throw std::runtime_error(                                                                                          \
        fmt::format(std::string{"Exception at {}:{} "} + MSG, __FILE__, __LINE__ EXCEPTION_VA_ARGS(__VA_ARGS__)))

#define EXCEPTION_NESTED(MSG, ...)                                                                                     \
    std::throw_with_nested(std::runtime_error(                                                                         \
        fmt::format(std::string{"Exception at {}:{} "} + MSG, __FILE__, __LINE__ EXCEPTION_VA_ARGS(__VA_ARGS__))))

#define BACKTRACE(EXP, MSG, ...)                                                                                       \
    logger.error(std::string{"Exception at {}:{} "} + MSG, __FILE__, __LINE__ EXCEPTION_VA_ARGS(__VA_ARGS__));         \
    Engine::backtrace(logger, EXP)
