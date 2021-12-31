#include "Worker.hpp"

#define CMP "Worker"

using namespace Scissio;

PeriodicWorker::PeriodicWorker(const std::chrono::milliseconds& period)
    : period(period), service(std::make_unique<asio::io_service>()),
      /*work(std::make_unique<asio::io_service::work>(*service)),*/ terminate(false), thread([this]() { run(); }) {
}

PeriodicWorker::~PeriodicWorker() {
    if (thread.joinable()) {
        {
            std::unique_lock<std::mutex> lock{mutex};
            terminate = true;
            cv.notify_all();
        }

        work.reset();
        thread.join();
    }
}

void PeriodicWorker::run() {
    while (true) {
        const auto t0 = std::chrono::steady_clock::now();
        service->reset();
        service->run();
        const auto t1 = std::chrono::steady_clock::now();

        const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);

        if (diff < period) {
            const auto toSleep = period - diff;
            std::unique_lock<std::mutex> lock{mutex};
            if (cv.wait_for(lock, toSleep, [&]() { return terminate; })) {
                break;
            }
        }
    }
}

BackgroundWorker::BackgroundWorker(size_t numThreads)
    : service(std::make_unique<asio::io_service>()), work(std::make_unique<asio::io_service::work>(*service)) {

    for (size_t i = 0; i < numThreads; i++) {
        threads.emplace_back([this]() { service->run(); });
    }
}

BackgroundWorker::~BackgroundWorker() {
    stop();
}

void BackgroundWorker::stop() {
    if (!threads.empty()) {
        work.reset();
        for (auto& thread : threads) {
            thread.join();
        }
        threads.clear();
    }
}

Worker::Worker(const int num) : service(std::make_unique<asio::io_service>()), flag{true} {
    for (auto i = 0; i < num; i++) {
        threads.emplace_back(&Worker::work, this);
    }
}

Worker::~Worker() {
    flag.store(false);
    cv.notify_all();

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();
}

void Worker::run() {
    service->restart();

    counter.store(threads.size());
    cv.notify_all();

    while (counter.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Worker::work() {
    Log::d(CMP, "Starting worker thread");
    while (flag.load()) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock);
        }

        if (!flag.load()) {
            break;
        }

        service->run();
        --counter;
    }
    Log::d(CMP, "Stopping worker thread");
}
