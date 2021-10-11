#pragma once

#include "NetworkServer.hpp"
#include "NetworkTcpAcceptor.hpp"

namespace Scissio::Network {
class SCISSIO_API TcpServer : public Server {
public:
    TcpServer(int port);
    virtual ~TcpServer() = default;
};
} // namespace Scissio::Network
