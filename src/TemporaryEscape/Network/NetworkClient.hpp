#pragma once

#include "../Utils/Log.hpp"
#include "NetworkAsio.hpp"
#include "NetworkServer.hpp"
#include <thread>

namespace Engine::Network {
class ENGINE_API Client {
public:
    Client(EventListener& listener);
    virtual ~Client();

    template <typename T> std::shared_ptr<T> connect(const std::string& address, const int port) {
        auto acceptor = std::make_shared<T>(listener, ecdhe, service, address, port);
        return acceptor;
    }

    template <typename T> void send(const T& message) {
        getStream().send(message);
    }

    virtual Stream& getStream() = 0;

private:
    void startIoService();
    void stopIoService();

    EventListener& listener;

    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    asio::ip::tcp::endpoint endpoint;
    std::thread thread;

    Crypto::Ecdhe ecdhe;
};
} // namespace Engine::Network
