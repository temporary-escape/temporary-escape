#pragma once

#include "NetworkServer.hpp"
#include "NetworkTcpAcceptor.hpp"

namespace Scissio::Network {
class SCISSIO_API TcpServer : public Server {
public:
    TcpServer(EventListener& listener, int port);
    virtual ~TcpServer();

private:
    std::shared_ptr<TcpAcceptor> acceptor;
};
} // namespace Scissio::Network
