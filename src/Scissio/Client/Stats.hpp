#pragma once

#include <atomic>

namespace Scissio {
struct Stats {
    std::atomic<uint64_t> packetsSent{0};
    std::atomic<uint64_t> packetsReceived{0};
    std::atomic<uint64_t> networkLatencyMs{0};
    std::atomic<uint64_t> serverLatencyMs{0};
    std::atomic<uint64_t> frameTimeMs{0};
};
} // namespace Scissio
