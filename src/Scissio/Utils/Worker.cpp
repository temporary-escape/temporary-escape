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

Worker::Worker(const int num) : start(num), service(std::make_unique<asio::io_service>()), flag{true}, counter(0) {
    for (auto i = 0; i < num; i++) {
        threads.emplace_back(&Worker::work, this);
    }

    // Wait for threads to enter the lock so we know the threads
    // are ready to accept work.
    std::unique_lock<std::mutex> lock(mutex);
    cvStop.wait(lock, [this]() { return start == 0; });
}

Worker::~Worker() {
    // Wait until all work has been completed.
    {
        std::unique_lock<std::mutex> lock(mutex);
        cvStop.wait(lock, [this]() { return counter == 0; });
    }

    // Set threads to stop and notify all of them
    flag.store(false);
    cvStart.notify_all();

    // Join the threads
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();
}

void Worker::run() {
    // Update the counter which we will use to know
    // whether the threads have completed their work.
    {
        std::unique_lock<std::mutex> lock(mutex);
        counter = threads.size();
    }

    // Start the service and notify threads.
    service->restart();
    cvStart.notify_all();

    // Wait for threads to complete their work.
    {
        std::unique_lock<std::mutex> lock(mutex);
        cvStop.wait(lock, [this]() { return counter == 0; });
    }
}

void Worker::work() {
    while (flag.load()) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            // Notify the constructor's wait
            if (start > 0) {
                --start;
                cvStop.notify_one();
            }
            // Wait for work from the run() function.
            cvStart.wait(lock);
        }

        // Shutdown?
        if (!flag.load()) {
            break;
        }

        // Complete all work, will not return until all work has been done.
        service->run();

        // Notify the run() function that our work has been completed.
        {
            std::unique_lock<std::mutex> lock(mutex);
            --counter;
            cvStop.notify_one();
        }
    }
}
