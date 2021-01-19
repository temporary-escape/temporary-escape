#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "NetworkStream.hpp"
#include "NetworkTcpStream.hpp"

namespace Scissio::Network {
class Client;

class SCISSIO_API TcpConnector : public Acceptor, public std::enable_shared_from_this<TcpConnector> {
public:
    explicit TcpConnector(Client& client, asio::io_service& service, const std::string& address, int port);
    virtual ~TcpConnector();

    void close();
    void start() override;
    void receive(const StreamPtr& stream, const Packet& packet) override;

private:
    Client& client;
    asio::ip::tcp::endpoint endpoint;
    std::shared_ptr<TcpStream> stream;
};
} // namespace Scissio::Network
