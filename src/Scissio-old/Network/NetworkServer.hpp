#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "Packet.hpp"
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace Scissio::Network {
class SCISSIO_API Server {
public:
    Server();
    virtual ~Server();

    template <typename T> void bind(const int port) {
        acceptor = std::make_shared<T>(*this, service, port);
        acceptor->start();
    }

    virtual void eventConnect(const StreamPtr& stream) = 0;
    virtual void eventDisconnect(const StreamPtr& stream) = 0;
    virtual void eventPacket(const StreamPtr& stream, Packet packet) = 0;

private:
    void startIoService();
    void stopIoService();

    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::thread thread;
    std::shared_mutex mutex;
    std::shared_ptr<Acceptor> acceptor;
};
} // namespace Scissio::Network
