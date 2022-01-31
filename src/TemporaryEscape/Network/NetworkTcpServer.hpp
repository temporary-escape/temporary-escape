#pragma once

#include "NetworkServer.hpp"
#include "NetworkTcpAcceptor.hpp"

namespace Engine::Network {
class ENGINE_API TcpServer : public Server {
public:
    TcpServer(EventListener& listener, int port);
    virtual ~TcpServer();

private:
    std::shared_ptr<TcpAcceptor> acceptor;
};
} // namespace Engine::Network
