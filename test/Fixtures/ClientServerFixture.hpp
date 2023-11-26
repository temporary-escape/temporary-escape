#include "../Common.hpp"
#include <Engine/Client/Client.hpp>
#include <Engine/Database/DatabaseRocksdb.hpp>
#include <Engine/Server/Server.hpp>
#include <Engine/Utils/Random.hpp>

namespace Engine {
class ClientServerFixture {
public:
    ClientServerFixture();

    ~ClientServerFixture();

    void startServer();
    void clientConnect();
    void clientDisconnect();

    Config config;
    TmpDir tmpDir;
    PlayerLocalProfile playerLocalProfile{};
    std::unique_ptr<AssetsManager> assetsManager;
    std::unique_ptr<Database> db;
    std::unique_ptr<Server> server;
    std::unique_ptr<Client> client;
    std::atomic_bool clientFlag;
    std::thread clientThread;
};
} // namespace Engine
