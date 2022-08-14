#pragma once

#include "../Future.hpp"
#include "../Library.hpp"
#include <functional>
#include <string>
#include <thread>

namespace Engine {
class ENGINE_API InitTask {
public:
    InitTask() = default;
    InitTask(const InitTask& other) = delete;
    InitTask(InitTask&& other) noexcept;
    explicit InitTask(std::string name, std::function<void()> callback);
    ~InitTask() = default;
    InitTask& operator=(const InitTask& other) = delete;
    InitTask& operator=(InitTask&& other) noexcept;
    void swap(InitTask& other) noexcept;
    void resolve();
    [[nodiscard]] bool isDone() const {
        return done;
    }
    void setOnDone(std::function<void()> fn) {
        onDone = std::move(fn);
    }
    void setReady() {
        ready = true;
    }

private:
    std::string name;
    std::function<void()> callback;
    std::function<void()> onDone;
    Future<void> future;
    bool ready{false};
    bool started{false};
    bool done{false};
};
} // namespace Engine
