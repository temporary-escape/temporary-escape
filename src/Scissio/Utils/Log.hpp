#pragma once

#include "../Library.hpp"
#include <fmt/format.h>
#include <iostream>
#include <utility>

namespace Scissio {
namespace Log {
extern SCISSIO_API void configure(bool debug);

extern SCISSIO_API void i(const std::string& cmp, const std::string& msg);
extern SCISSIO_API void w(const std::string& cmp, const std::string& msg);
extern SCISSIO_API void e(const std::string& cmp, const std::string& msg);
extern SCISSIO_API void d(const std::string& cmp, const std::string& msg);

template <typename... Args> void i(const std::string& cmp, const std::string& msg, Args&&... args) {
    i(cmp, fmt::format(msg, std::forward<Args>(args)...));
}

template <typename... Args> void w(const std::string& cmp, const std::string& msg, Args&&... args) {
    w(cmp, fmt::format(msg, std::forward<Args>(args)...));
}

template <typename... Args> void e(const std::string& cmp, const std::string& msg, Args&&... args) {
    e(cmp, fmt::format(msg, std::forward<Args>(args)...));
}

template <typename... Args> void d(const std::string& cmp, const std::string& msg, Args&&... args) {
    d(cmp, fmt::format(msg, std::forward<Args>(args)...));
}
} // namespace Log
} // namespace Scissio
