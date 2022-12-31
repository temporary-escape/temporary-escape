#include "async_loader.hpp"

using namespace Engine;

AsyncLoader::Synchronizer::Synchronizer(AsyncLoader& loader) : loader{loader} {
}

void AsyncLoader::Synchronizer::operator()(std::function<void()>&& fn) {
    loader.synchronize(std::move(fn));
}

AsyncLoader::~AsyncLoader() {
    stop();
}

void AsyncLoader::synchronize(std::function<void()>&& fn) {
    {
        std::lock_guard<std::mutex> lock{mutex};
        synchronized = std::move(fn);
        ready = true;
    }

    {
        std::unique_lock<std::mutex> lock{mutex};
        cv.wait(lock, [this]() { return processed || stopToken.load(); });
    }
}

void AsyncLoader::push(std::function<void(Synchronizer&)>&& job) {
    jobs.push_back(std::move(job));
}

void AsyncLoader::start() {
    stopToken = false;
    future = std::async([this]() { run(); });
}

void AsyncLoader::run() {
    Synchronizer synchronizer{*this};

    for (auto& job : jobs) {
        if (stopToken.load()) {
            break;
        }
        job(synchronizer);
    }
}

void AsyncLoader::stop() {
    stopToken = true;
    if (future.valid()) {
        future.get();
    }
}

void AsyncLoader::pool() {
    if (!future.valid()) {
        return;
    }

    if (future.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready) {
        future.get();
        return;
    }

    {
        std::unique_lock<std::mutex> lock{mutex};
        if (!ready) {
            return;
        }

        synchronized();
        ready = false;
        processed = true;
    }
    cv.notify_one();
}