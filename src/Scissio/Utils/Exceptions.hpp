#pragma once

#include "Log.hpp"

#include <filesystem>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace Scissio {

inline void backtrace(const std::exception& e, const size_t level = 0) {
    Log::e("{} {}", std::string(level * 2, ' '), e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& ex) {
        backtrace(ex, level + 1);
    } catch (...) {
    }
}
} // namespace Scissio

#define __FILENAME__ std::filesystem::path(__FILE__).stem().string()
#define WHERE (__FILENAME__ + std::string(":") + std::to_string(__LINE__))
#define EXCEPTION(...) throw std::runtime_error(fmt::format("Exception at {} {}", WHERE, fmt::format(__VA_ARGS__)));
#define EXCEPTION_NESTED(...)                                                                                          \
    std::throw_with_nested(std::runtime_error(fmt::format("Exception at {} {}", WHERE, fmt::format(__VA_ARGS__))));
