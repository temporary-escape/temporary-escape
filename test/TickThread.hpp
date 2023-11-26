#pragma once

#include <atomic>
#include <functional>
#include <thread>

namespace Engine {
class TickThread {
public:
    explicit TickThread(std::chrono::milliseconds ms, std::function<void()>&& tick) : flag(true) {
        thread = std::thread([this, ms, tick = std::move(tick)] {
            while (flag.load()) {
                const auto t0 = std::chrono::steady_clock::now();
                tick();
                const auto t1 = std::chrono::steady_clock::now();
                const auto diff = t1 - t0;
                if (diff < ms) {
                    std::this_thread::sleep_for(ms - diff);
                }
            }
        });
    }

    ~TickThread() {
        flag.store(false);
        thread.join();
    }

private:
    std::atomic_bool flag;
    std::thread thread;
};
} // namespace Engine
