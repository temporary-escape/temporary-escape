#pragma once

#include "../Utils/Crypto.hpp"
#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "Packet.hpp"
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace Engine::Network {
class ENGINE_API Server {
public:
    Server(EventListener& listener);
    virtual ~Server();

    template <typename T> std::shared_ptr<T> bind(const int port) {
        auto acceptor = std::make_shared<T>(listener, ecdhe, service, port);
        acceptor->start();
        return acceptor;
    }

private:
    void startIoService();
    void stopIoService();

    EventListener& listener;

    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::thread thread;

    Crypto::Ecdhe ecdhe;
};
} // namespace Engine::Network
