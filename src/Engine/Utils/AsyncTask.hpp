#pragma once

#include "../Library.hpp"
#include <atomic>
#include <functional>
#include <future>
#include <thread>

namespace Engine {
template <typename T> class AsyncTask {
public:
    explicit AsyncTask(std::function<T(AsyncTask&)>&& fn) {
        future = std::async([fn = std::move(fn), this]() { fn(*this); });
    }

    ~AsyncTask() {
        if (future.valid()) {
            cancel();
            get();
        }
    }

    void cancel() {
        flag = true;
    }

    bool wait() {
        if (future.valid()) {
            if (future.template wait_for(std::chrono::milliseconds(1)) == std::future_status::ready) {
                return true;
            }
        }
        return false;
    }

    T get() {
        return future.get();
    }

private:
    std::future<T> future;
    std::atomic<bool> flag{false};
};
} // namespace Engine
