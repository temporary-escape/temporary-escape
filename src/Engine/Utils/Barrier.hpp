#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

namespace Engine {
// Based on: https://stackoverflow.com/a/27118537
class Barrier {
public:
    explicit Barrier(const size_t count) : threshold{count}, count{count}, generation{0} {
    }

    void wait() {
        std::unique_lock<std::mutex> lock{mMutex};
        auto g = generation;
        if (!--count) {
            generation++;
            count = threshold;
            cv.notify_all();
        } else {
            cv.wait(lock, [this, g] { return g != generation; });
        }
    }

private:
    std::mutex mMutex;
    std::condition_variable cv;
    std::size_t threshold;
    std::size_t count;
    std::size_t generation;
};
} // namespace Engine
