#include "FixtureClientServer.hpp"

FixtureClientServer::FixtureClientServer() {
    config.serverPassword = "password";
    config.userdataPath = tmpDir.value();
    config.userdataSavesPath = config.userdataPath / "Saves";

    const auto dbDir = config.userdataSavesPath / "Test";
    Fs::create_directories(dbDir);

    textureCompressor = std::make_unique<TextureCompressor>(NO_CREATE);
    canvas = std::make_unique<Canvas2D>(NO_CREATE);
    assetManager = std::make_unique<AssetManager>(config, *canvas, *textureCompressor);
    modManager = std::make_unique<ModManager>();
    db = std::make_unique<Database>(dbDir);
    server = std::make_unique<Server>(config, *assetManager, *db);
}

void FixtureClientServer::connectToServer() {
    client = std::make_unique<Client>(config, "localhost", config.serverPort);
}
