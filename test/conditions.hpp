#pragma once

#include <chrono>
#include <functional>
#include <thread>

namespace Engine {
static inline bool waitForCondition(const std::function<bool()>& fn, int wait = 2) {
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds{wait};

    while (std::chrono::steady_clock::now() < timeout) {
        if (fn()) {
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return false;
}

#define REQUIRE_EVENTUALLY(expr) REQUIRE(waitForCondition([&]() { return (expr); }))
#define REQUIRE_EVENTUALLY_S(expr, sec) REQUIRE(waitForCondition([&]() { return (expr); }, sec))
} // namespace Engine
