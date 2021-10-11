#pragma once

#include "../Library.hpp"
#include <fmt/format.h>
#include <utility>

namespace Scissio {
class SCISSIO_API Log {
public:
    enum class Type { E, W, D, V, I };

    template <typename... Args> static void e(const std::string& str, Args&&... args) {
        e(fmt::format(str, std::forward<Args>(args)...));
    }
    template <typename... Args> static void w(const std::string& str, Args&&... args) {
        w(fmt::format(str, std::forward<Args>(args)...));
    }
    template <typename... Args> static void d(const std::string& str, Args&&... args) {
        d(fmt::format(str, std::forward<Args>(args)...));
    }
    template <typename... Args> static void v(const std::string& str, Args&&... args) {
        v(fmt::format(str, std::forward<Args>(args)...));
    }
    template <typename... Args> static void i(const std::string& str, Args&&... args) {
        i(fmt::format(str, std::forward<Args>(args)...));
    }

    static void e(const std::string& str);
    static void w(const std::string& str);
    static void d(const std::string& str);
    static void v(const std::string& str);
    static void i(const std::string& str);
    static void print(Type type, const std::string& str);
};
} // namespace Scissio
