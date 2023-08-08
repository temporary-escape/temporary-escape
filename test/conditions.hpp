#pragma once

#include <chrono>
#include <functional>
#include <thread>

namespace Engine {
static inline bool waitForCondition(const std::function<bool()>& fn) {
    const auto start = std::chrono::steady_clock::now();

    while (true) {
        if (fn()) {
            return true;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto test = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        if (test > 2000) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
} // namespace Engine
