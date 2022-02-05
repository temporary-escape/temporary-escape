#include "../Common.hpp"
#include <TemporaryEscape/Modding/ModManager.hpp>
#include <TemporaryEscape/Client/Client.hpp>
#include <TemporaryEscape/Server/Server.hpp>

class FixtureClientServer {
public:
    FixtureClientServer();

    void connectToServer();

    TmpDir tmpDir;
    Config config;
    std::unique_ptr<TextureCompressor> textureCompressor;
    std::unique_ptr<Canvas2D> canvas;
    std::unique_ptr<ModManager> modManager;
    std::unique_ptr<AssetManager> assetManager;
    std::unique_ptr<Database> db;
    std::unique_ptr<Server> server;
    std::unique_ptr<Client> client;
};
