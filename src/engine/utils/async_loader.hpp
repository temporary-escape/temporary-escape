#pragma once

#include "../library.hpp"
#include <condition_variable>
#include <functional>
#include <future>
#include <list>

namespace Engine {
class ENGINE_API AsyncLoader {
public:
    class Synchronizer {
    public:
        explicit Synchronizer(AsyncLoader& loader);

        void operator()(std::function<void()>&& fn);

    private:
        AsyncLoader& loader;
    };

    AsyncLoader() = default;
    ~AsyncLoader();

    void synchronize(std::function<void()>&& fn);
    void push(std::function<void(Synchronizer&)>&& job);
    void start();
    void run();
    void stop();
    void pool();

private:
    std::future<void> future;
    std::list<std::function<void(Synchronizer&)>> jobs;
    std::atomic<bool> stopToken;
    std::mutex mutex;
    std::function<void()> synchronized;
    bool processed{false};
    bool ready{false};
    std::condition_variable cv;
};

} // namespace Engine
