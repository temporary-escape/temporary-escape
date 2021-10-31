#pragma once

#include "NetworkClient.hpp"
#include "NetworkTcpConnector.hpp"

namespace Scissio::Network {
class SCISSIO_API TcpClient : public Client {
public:
    explicit TcpClient(const std::string& address, int port);
    virtual ~TcpClient() = default;
};
} // namespace Scissio::Network
