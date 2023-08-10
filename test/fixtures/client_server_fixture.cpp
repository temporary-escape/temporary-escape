#include "client_server_fixture.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ClientServerFixture::ClientServerFixture() {
    config.assetsPath = Path{ROOT_DIR} / "assets";

    assetsManager = std::make_unique<AssetsManager>(config);
    for (auto& loadFn : assetsManager->getLoadQueue()) {
        loadFn(nullptr, nullptr);
    }

    DatabaseRocksDB::Options options{};
    options.cacheSizeMb = config.server.dbCacheSize;
    options.debugLogging = config.server.dbDebug;
    options.compression = config.server.dbCompression;
    db = std::make_unique<DatabaseRocksDB>(tmpDir.value(), DatabaseRocksDB::Options{});

    server = std::make_unique<Server>(config, *assetsManager, *db);

    playerLocalProfile.name = "Test Player";
    playerLocalProfile.secret = 112233445566ULL;
}

ClientServerFixture::~ClientServerFixture() {
    clientDisconnect();
}

void ClientServerFixture::clientConnect() {
    client =
        std::make_unique<Client>(config, *assetsManager, playerLocalProfile, nullptr, "localhost", config.serverPort);
    clientFlag.store(true);
    clientThread = std::thread([this]() {
        while (clientFlag.load()) {
            client->update();
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
