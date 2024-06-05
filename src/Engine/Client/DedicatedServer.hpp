#pragma once

#include "../Config.hpp"
#include "../Network/NetworkUdpClient.hpp"
#include "../Network/NetworkUdpServer.hpp"
#include "../Server/MatchmakerClient.hpp"
#include <list>
#include <thread>

namespace Engine {
class ENGINE_API DedicatedServer : public NetworkDispatcher2 {
public:
    explicit DedicatedServer(Config& config);
    ~DedicatedServer();

    void wait();

private:
    void stop();

public:
    /*void onMatchmakerConnect() override;
    void onMatchmakerDisconnect() override;*/

private:
    Config& config;
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::list<std::thread> threads;
    std::unique_ptr<NetworkUdpServer> server;
    std::unique_ptr<MatchmakerClient> matchmaker;
    std::string publicId;
};
} // namespace Engine
