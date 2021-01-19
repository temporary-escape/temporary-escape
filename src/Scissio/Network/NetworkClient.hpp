#pragma once

#include "../Utils/Log.hpp"
#include "NetworkAsio.hpp"
#include "NetworkServer.hpp"
#include "NetworkSession.hpp"
#include <thread>

namespace Scissio::Network {
class SCISSIO_API Client : public Session {
public:
    explicit Client(uint64_t sessionId, const std::string& address);
    virtual ~Client();

    void receive(const StreamPtr& stream, const Packet& packet);
    void close();

    template <typename T> void initConnection(const int port, const int64_t playerId, std::string password) {
        add<T>(port);

        MessageSessionInit msg;
        msg.password = std::move(password);
        msg.playerId = playerId;
        send(getStreams().size() - 1, msg);

        waitForSession();
    }

    template <typename T> void addConnection(const int port) {
        add<T>(port);

        const auto idx = getStreams().size() - 1;
        MessageSessionConnect msg;
        msg.sessionId = getSessionId();
        msg.state = idx;
        send(idx, msg);

        waitForSession();
        if (state != idx) {
            EXCEPTION("Server responded with invalid state");
        }
    }

    virtual void dispatch(const Packet& packet) = 0;

private:
    void waitForSession();

    template <typename T> void add(const int port) {
        std::lock_guard<std::mutex> lock{mutex};
        acceptors.push_back(std::make_shared<T>(*this, service, address, port));
        acceptors.back()->start();
    }

    void startIoService();
    void stopIoService();

    std::string address;
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    asio::ip::tcp::endpoint endpoint;
    std::thread thread;

    std::mutex mutex;
    std::condition_variable cvConnection;
    std::string error;
    uint64_t state;
    std::vector<std::shared_ptr<Acceptor>> acceptors;
};
} // namespace Scissio::Network
