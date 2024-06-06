#pragma once

#include "../Config.hpp"
#include "../Network/NetworkUdpClient.hpp"
#include "../Network/NetworkUdpServer.hpp"
#include "../Server/MatchmakerClient.hpp"
#include <list>
#include <thread>

namespace Engine {
class ENGINE_API Server;
class ENGINE_API AssetsManager;

class ENGINE_API DedicatedServer : public NetworkDispatcher2 {
public:
    explicit DedicatedServer(Config& config);
    ~DedicatedServer();

    void wait();

private:
    void stop();

    Config& config;
    MatchmakerClient matchmakerClient;
    std::unique_ptr<AssetsManager> assetsManager;
    std::unique_ptr<Server> server;
};
} // namespace Engine
