#pragma once

#include <atomic>

namespace Scissio {
struct Stats {
    struct Network {
        std::atomic<uint64_t> packetsSent{0};
        std::atomic<uint64_t> packetsReceived{0};
        std::atomic<uint64_t> latencyMs{0};
    } network;

    struct Render {
        std::atomic<uint64_t> frameTimeMs{0};
    } render;
};
} // namespace Scissio
