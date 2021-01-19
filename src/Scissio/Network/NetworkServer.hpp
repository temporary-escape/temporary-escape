#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "NetworkMessageAcceptor.hpp"
#include "NetworkSession.hpp"
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace Scissio::Network {
class SCISSIO_API Server {
public:
    Server();
    virtual ~Server();

    template <typename T> void addAcceptor(const int port) {
        acceptors.push_back(std::make_shared<T>(*this, service, port));
        acceptors.back()->start();
    }

    void receive(const StreamPtr& stream, const Packet& packet);
    virtual SessionPtr createSession(int64_t playerId, const std::string& password) = 0;
    virtual void acceptSession(const SessionPtr& session) = 0;
    virtual void dispatch(const SessionPtr& session, const Packet& packet) = 0;

private:
    void startIoService();
    void stopIoService();

    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::thread thread;
    std::shared_mutex mutex;
    std::vector<std::shared_ptr<Acceptor>> acceptors;
    std::unordered_map<uint64_t, SessionWeak> sessions;
};

struct SCISSIO_API SessionInfo {
    int port{0};
    bool isTcp{false};

    MSGPACK_DEFINE(port, isTcp);
};

struct SCISSIO_API MessageSessionInit {
    MESSAGE_REGISTER(MessageSessionInit);

    int64_t playerId{0};
    std::string password;

    MSGPACK_DEFINE(playerId, password);
};

struct SCISSIO_API MessageSessionInitResponse {
    MESSAGE_REGISTER(MessageSessionInitResponse);

    uint64_t sessionId{0};
    std::string error;

    MSGPACK_DEFINE(sessionId, error);
};

struct SCISSIO_API MessageSessionConnect {
    MESSAGE_REGISTER(MessageSessionConnect);

    uint64_t sessionId{0};
    uint64_t state{0};

    MSGPACK_DEFINE(sessionId, state);
};
} // namespace Scissio::Network
