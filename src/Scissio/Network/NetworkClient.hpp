#pragma once

#include "../Utils/Log.hpp"
#include "NetworkAsio.hpp"
#include "NetworkServer.hpp"
#include <thread>

namespace Scissio::Network {
class SCISSIO_API Client {
public:
    explicit Client();
    virtual ~Client();

    template <typename T> void connect(const std::string& address, const int port) {
        acceptor = std::make_shared<T>(*this, service, address, port);
    }

    template <typename T> void send(const T& message) {
        if (!stream) {
            EXCEPTION("Client is not connected to any server");
        }
        stream->send(message);
    }

    void eventConnect(const StreamPtr& stream);
    void eventDisconnect(const StreamPtr& stream);
    virtual void eventPacket(const StreamPtr& stream, Packet packet) = 0;

private:
    void startIoService();
    void stopIoService();

    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    asio::ip::tcp::endpoint endpoint;
    std::thread thread;

    std::mutex mutex;
    std::condition_variable cvConnection;
    std::string error;
    uint64_t state;
    std::shared_ptr<Acceptor> acceptor;
    StreamPtr stream;
};
} // namespace Scissio::Network
