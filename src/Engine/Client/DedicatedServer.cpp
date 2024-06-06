#include "DedicatedServer.hpp"
#include "../Assets/AssetsManager.hpp"
#include "../Server/Server.hpp"
#include <csignal>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

volatile std::sig_atomic_t signalStatus{0};

static void signalHandler(const int signal) {
    signalStatus = signal;
}

DedicatedServer::DedicatedServer(Config& config) : config{config}, matchmakerClient{config} {
    assetsManager = std::make_unique<AssetsManager>(config);

    const auto loadQueue = assetsManager->getLoadQueue();
    for (auto& loadFn : loadQueue) {
        loadFn(nullptr, nullptr);
    }

    Server::Options options{};
    options.seed = 123456789ULL;
    options.savePath = config.userdataSavesPath / "Default";
    options.name = "Dedicated Server";
    options.password = "";

    server = std::make_unique<Server>(config, *assetsManager, options, &matchmakerClient);
}

DedicatedServer::~DedicatedServer() {
    stop();
}

void DedicatedServer::stop() {
    logger.info("Stopping dedicated server");

    server.reset();
    assetsManager.reset();
    matchmakerClient.close();
}

void DedicatedServer::wait() {
    logger.warn("Press CTRL+C to stop the server!");
    std::signal(SIGINT, signalHandler);
    while (signalStatus == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
}
