#include "ClientServerFixture.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ClientServerFixture::ClientServerFixture() {
    config.assetsPath = Path{ROOT_DIR} / "assets";

    playerLocalProfile.name = "Test Player";
    playerLocalProfile.secret = 112233445566ULL;
}

ClientServerFixture::~ClientServerFixture() {
    clientDisconnect();
}

void ClientServerFixture::startServer() {
    assetsManager = std::make_unique<AssetsManager>(config);
    for (auto& loadFn : assetsManager->getLoadQueue()) {
        loadFn(nullptr, nullptr);
    }

    Server::Options options{};
    options.seed = 123456789ULL;
    options.savePath = tmpDir.value();

    server = std::make_unique<Server>(config, *assetsManager, options);
}

void ClientServerFixture::clientConnect() {
    client = std::make_unique<Client>(
        config, *assetsManager, playerLocalProfile, nullptr, "localhost", config.network.serverPort);
    clientFlag.store(true);
    clientThread = std::thread([this]() {
        while (clientFlag.load()) {
            client->update(0.1f);
            std::this_thread::sleep_for(std::chrono::milliseconds{16});
        }
    });
}

void ClientServerFixture::clientDisconnect() {
    clientFlag.store(false);
    if (clientThread.joinable()) {
        clientThread.join();
    }
    client.reset();
}
