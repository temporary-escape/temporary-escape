#include "../Common.hpp"
#include <TemporaryEscape/Client/Client.hpp>
#include <TemporaryEscape/Modding/ModManager.hpp>
#include <TemporaryEscape/Server/Server.hpp>

class FixtureClientServer {
public:
    FixtureClientServer();

    std::shared_ptr<Client> newClient(const std::string& playerName);

    TmpDir tmpDir;
    Config config;
    std::unique_ptr<TextureCompressor> textureCompressor;
    std::unique_ptr<Canvas2D> canvas;
    std::unique_ptr<ModManager> modManager;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<TransactionalDatabase> db;
    std::unique_ptr<Server> server;
    std::unique_ptr<Stats> stats;
    // std::unique_ptr<Client> client;
    std::list<std::weak_ptr<Client>> clients;
    std::mutex clientMutex;
    std::unique_ptr<TickThread> clientThread;
};
