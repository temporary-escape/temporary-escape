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

    try {
        server = std::make_unique<NetworkUdpServer>(config, service);
        matchmaker = std::make_unique<Matchmaker>(config.network.matchmakerUrl);
        matchmaker->registerServerAndListen("Some server name", *server);
    } catch (...) {
        stop();
        throw;
    }
}

DedicatedServer::~DedicatedServer() {
    stop();
}

void DedicatedServer::stop() {
    if (matchmaker) {
        logger.info("Stopping matchmaker");
        matchmaker->close();
    }

    if (server) {
        logger.info("Stopping server");
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

    matchmaker.reset();
    server.reset();
}

void DedicatedServer::wait() {
    asio::signal_set signals(service, SIGINT, SIGTERM);
    auto future = signals.async_wait(asio::use_future);
    const auto res = future.get();

    logger.info("Received signal: {}", res);
    // std::this_thread::sleep_for(std::chrono::seconds{1});
    stop();
}

/*void DedicatedServer::onMatchmakerConnect() {
    server->stunRequest([this](NetworkStunClient::Result result) {
        logger.info("Got STUN response endpoint: {}", result.endpoint);
        matchmaker->serverRegister("Some Server Name", result.endpoint);
    });
}

void DedicatedServer::onMatchmakerDisconnect() {
}*/
