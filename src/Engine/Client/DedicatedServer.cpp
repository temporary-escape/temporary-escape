#include "DedicatedServer.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

DedicatedServer::DedicatedServer(Config& config) : config{config} {
    work = std::make_unique<asio::io_service::work>(service);
    for (auto i = 0; i < 4; i++) {
        threads.emplace_back([this]() {
            try {
                logger.debug("Started server thread");
                service.run();
                logger.debug("Stopped server thread");
            } catch (std::exception& e) {
                BACKTRACE(e, "Exception caught in DedicatedServer thread");
            }
        });
    }

    server = std::make_unique<NetworkUdpServer>(config, service);
}

DedicatedServer::~DedicatedServer() {
    stop();
}

void DedicatedServer::stop() {
    if (server) {
        logger.info("Closing server");
        server->stop();
    }

    if (work) {
        logger.info("Stopping io_service");
        work.reset();
        service.stop();
    }

    if (!threads.empty()) {
        logger.info("Joining threads");
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }
}

void DedicatedServer::wait() {
    asio::signal_set signals(service, SIGINT, SIGTERM);
    auto future = signals.async_wait(asio::use_future);
    const auto res = future.get();

    logger.info("Received signal: {}", res);
    stop();
}
