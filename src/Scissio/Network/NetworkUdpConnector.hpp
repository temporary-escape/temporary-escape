#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "NetworkStream.hpp"
#include "NetworkUdpStream.hpp"

namespace Scissio::Network {
class Client;

class SCISSIO_API UdpConnector : public Acceptor, public std::enable_shared_from_this<UdpConnector> {
public:
    explicit UdpConnector(Client& client, asio::io_service& service, const std::string& address, int port);
    virtual ~UdpConnector();

    void close();
    void start() override;
    void receive(const StreamPtr& stream, Packet packet) override;

private:
    void accept();

    Client& client;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint endpoint;
    std::shared_ptr<UdpStream> stream;
    msgpack::unpacker unp;
};
} // namespace Scissio::Network
