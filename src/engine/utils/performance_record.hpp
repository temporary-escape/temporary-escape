#pragma once

#include "../library.hpp"
#include <atomic>
#include <chrono>
#include <vector>

namespace Engine {
class ENGINE_API PerformanceRecord {
public:
    explicit PerformanceRecord();
    void update(std::chrono::nanoseconds value);
    std::chrono::nanoseconds value() const {
        return current.load();
    }

private:
    std::vector<std::chrono::nanoseconds> recorded;
    std::atomic<std::chrono::nanoseconds> current;
    std::chrono::steady_clock::time_point lastTimePoint;
};
} // namespace Engine
