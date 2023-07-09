#include "performance_record.hpp"

using namespace Engine;

PerformanceRecord::PerformanceRecord() : lastTimePoint{std::chrono::steady_clock::now()} {
}

void PerformanceRecord::update(const std::chrono::nanoseconds value) {
    recorded.push_back(value);
    const auto now = std::chrono::steady_clock::now();
    if (now - lastTimePoint > std::chrono::seconds{1}) {
        uint64_t sum{0};
        for (const auto& rec : recorded) {
            sum += rec.count();
        }
        current = std::chrono::nanoseconds{sum / recorded.size()};
        recorded.clear();
        lastTimePoint = now;
    }
}
