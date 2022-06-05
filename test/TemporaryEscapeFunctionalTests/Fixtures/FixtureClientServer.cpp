#include "FixtureClientServer.hpp"
#include <TemporaryEscape/Utils/Random.hpp>
#include <TemporaryEscape/Utils/RocksDB.hpp>

FixtureClientServer::FixtureClientServer() {
    config.assetsPath = Path(ROOT_DIR) / "assets";
    config.wrenPaths = {config.assetsPath};
    config.userdataPath = tmpDir.value();
    config.userdataSavesPath = config.userdataPath / "Saves";

    const auto dbDir = config.userdataSavesPath / "Test";
    Fs::create_directories(dbDir);

    textureCompressor = std::make_unique<TextureCompressor>(NO_CREATE);
    canvas = std::make_unique<Canvas2D>(NO_CREATE);
    assetManager = std::make_unique<AssetManager>(config, *canvas, *textureCompressor);
    modManager = std::make_unique<ModManager>();
    modManager->load(*assetManager, config.assetsPath / "base");

    auto loadQueue = assetManager->getLoadQueue(true);
    while (!loadQueue.empty()) {
        loadQueue.front()();
        loadQueue.pop();
    }

    db = std::make_unique<RocksDB>(dbDir);
    server = std::make_unique<Server>(config, *modManager, *assetManager, *db);
    server->load();

    stats = std::make_unique<Stats>();

    clientThread = std::make_unique<TickThread>(std::chrono::milliseconds(50), [this]() {
        std::lock_guard<std::mutex> lock{clientMutex};
        auto it = clients.begin();
        while (it != clients.end()) {
            auto ptr = it->lock();
            if (!ptr) {
                clients.erase(it++);
            } else {
                ptr->update();
                ++it;
            }
        }
    });
}

std::shared_ptr<Client> FixtureClientServer::newClient(const std::string& playerName) {
    const auto profilePath = config.userdataPath / Path(playerName + ".yml");
    PlayerLocalProfile localProfile{};
    localProfile.secret = randomId();
    localProfile.name = playerName;
    localProfile.toYaml(profilePath);

    auto client = std::make_shared<Client>(config, *modManager, *stats, "localhost", config.serverPort, profilePath);

    {
        std::lock_guard<std::mutex> lock{clientMutex};
        clients.push_back(client);
    }

    waitForCondition([&]() { return client->getScene() != nullptr; });

    return client;
}
