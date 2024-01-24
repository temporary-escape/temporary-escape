#pragma once

#include "../Config.hpp"
#include "../Network/NetworkUdpClient.hpp"
#include "../Network/NetworkUdpServer.hpp"
#include <list>
#include <thread>

namespace Engine {
class ENGINE_API DedicatedServer {
public:
    DedicatedServer(Config& config);
    ~DedicatedServer();

    void wait();

private:
    void stop();

    Config& config;
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::list<std::thread> threads;
    std::unique_ptr<NetworkUdpServer> server;
};
} // namespace Engine
