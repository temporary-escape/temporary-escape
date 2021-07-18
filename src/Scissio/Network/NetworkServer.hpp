#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "NetworkSession.hpp"
#include "Packet.hpp"
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

    void receive(const StreamPtr& stream, Packet packet);
    virtual SessionPtr createSession(uint64_t uid, const std::string& name, const std::string& password) = 0;
    virtual void acceptSession(const SessionPtr& session) = 0;
    virtual void dispatch(const SessionPtr& session, Packet packet) = 0;

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

struct SCISSIO_API MessageSessionInit {
    uint64_t uid{0};
    std::string name;
    std::string password;

    MSGPACK_DEFINE(uid, name, password);
};

REGISTER_MESSAGE(MessageSessionInit);

struct SCISSIO_API MessageSessionInitResponse {
    uint64_t sessionId{0};
    std::string error;

    MSGPACK_DEFINE(sessionId, error);
};

REGISTER_MESSAGE(MessageSessionInitResponse);

struct SCISSIO_API MessageSessionConnect {
    uint64_t sessionId{0};
    uint64_t state{0};

    MSGPACK_DEFINE(sessionId, state);
};

REGISTER_MESSAGE(MessageSessionConnect);
} // namespace Scissio::Network
