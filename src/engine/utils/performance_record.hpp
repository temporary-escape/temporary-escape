#pragma once

#include "../library.hpp"
#include <chrono>
#include <vector>

namespace Engine {
class ENGINE_API PerformanceRecord {
public:
    explicit PerformanceRecord();
    void update(std::chrono::nanoseconds value);
    const std::chrono::nanoseconds& value() const {
        return current;
    }

private:
    std::vector<std::chrono::nanoseconds> recorded;
    std::chrono::nanoseconds current{0};
    std::chrono::steady_clock::time_point lastTimePoint;
};
} // namespace Engine
