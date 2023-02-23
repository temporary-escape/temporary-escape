#pragma once

#include "../library.hpp"
#include <fmt/format.h>
#include <iostream>
#include <optional>
#include <spdlog/spdlog.h>
#include <utility>

namespace Engine {
namespace Log {
extern ENGINE_API void i(const std::string& cmp, const std::string& msg);
extern ENGINE_API void w(const std::string& cmp, const std::string& msg);
extern ENGINE_API void e(const std::string& cmp, const std::string& msg);
extern ENGINE_API void d(const std::string& cmp, const std::string& msg);

template <typename... Args> void i(const std::string& cmp, const std::string& msg, Args&&... args) {
    Log::i(cmp, fmt::format(msg, std::forward<Args>(args)...));
}

template <typename... Args> void w(const std::string& cmp, const std::string& msg, Args&&... args) {
    Log::w(cmp, fmt::format(msg, std::forward<Args>(args)...));
}

template <typename... Args> void e(const std::string& cmp, const std::string& msg, Args&&... args) {
    Log::e(cmp, fmt::format(msg, std::forward<Args>(args)...));
}

template <typename... Args> void d(const std::string& cmp, const std::string& msg, Args&&... args) {
    Log::d(cmp, fmt::format(msg, std::forward<Args>(args)...));
}
} // namespace Log
} // namespace Engine
