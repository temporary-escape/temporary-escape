#pragma once

#include "NetworkClient.hpp"
#include "NetworkTcpConnector.hpp"

namespace Scissio::Network {
class SCISSIO_API TcpClient : public Client {
public:
    TcpClient(EventListener& listener, const std::string& address, int port);
    virtual ~TcpClient();

    Stream& getStream() override {
        return acceptor->getStream();
    }

private:
    std::shared_ptr<TcpConnector> acceptor;
};
} // namespace Scissio::Network
