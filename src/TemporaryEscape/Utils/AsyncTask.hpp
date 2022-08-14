#pragma once

#include "../Future.hpp"
#include "../Library.hpp"
#include <functional>
#include <string>
#include <thread>

namespace Engine {
class ENGINE_API AsyncTask {
public:
    AsyncTask() = default;
    AsyncTask(const AsyncTask& other) = delete;
    AsyncTask(AsyncTask&& other) noexcept;
    explicit AsyncTask(std::function<void()> callback);
    ~AsyncTask() = default;
    AsyncTask& operator=(const AsyncTask& other) = delete;
    AsyncTask& operator=(AsyncTask&& other) noexcept;
    void swap(AsyncTask& other) noexcept;
    void resolve();
    [[nodiscard]] bool isDone() const {
        return done;
    }

private:
    std::function<void()> callback;
    Future<void> future;
    bool done{false};
};
} // namespace Engine
