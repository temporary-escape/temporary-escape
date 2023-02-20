#pragma once

#include <chrono>
#include <functional>

namespace Engine {
static inline bool waitForCondition(const std::function<bool()>& fn) {
    const auto start = std::chrono::steady_clock::now();

    while (true) {
        if (fn()) {
            return true;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (test > 1000) {
            return false;
        }
    }
}
} // namespace Engine
