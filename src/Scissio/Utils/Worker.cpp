#include "Worker.hpp"

#define CMP "Worker"

using namespace Scissio;

Service::Service()
    : service(std::make_unique<asio::io_service>()), work(std::make_unique<asio::io_service::work>(*service)),
      thread([this]() { service->run(); }) {
}

Service::~Service() {
    if (thread.joinable()) {
        work.reset();
        thread.join();
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
