#pragma once

#include <asio.hpp>

namespace Engine {
class IoServiceRunner {
public:
    explicit IoServiceRunner(asio::io_service& service) : service(service) {
        work = std::make_unique<asio::io_service::work>(service);
        thread = std::thread([this]() { this->service.run(); });
    }

    ~IoServiceRunner() {
        stop();
    }

    void stop() {
        work.reset();
        thread.join();
    }

    asio::io_service& service;
    std::unique_ptr<asio::io_service::work> work;
    std::thread thread;
};
} // namespace Engine
