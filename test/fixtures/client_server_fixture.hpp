#include "../common.hpp"
#include <engine/client/client.hpp>
#include <engine/database/database_rocksdb.hpp>
#include <engine/server/server.hpp>
#include <engine/utils/random.hpp>

namespace Engine {
class ClientServerFixture {
public:
    ClientServerFixture();

    ~ClientServerFixture();

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
