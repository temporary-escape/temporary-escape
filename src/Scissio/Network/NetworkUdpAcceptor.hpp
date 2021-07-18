#pragma once

// clang-format off
#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"
#include "NetworkStream.hpp"
#include "../Utils/Msgpack.hpp"
// clang-format on

namespace Scissio::Network {
class Server;
class UdpStream;

class SCISSIO_API UdpAcceptor : public Acceptor, public std::enable_shared_from_this<UdpAcceptor> {
public:
    explicit UdpAcceptor(Server& server, asio::io_service& service, int port);
    virtual ~UdpAcceptor();

    void close();
    void start() override;
    void receive(const StreamPtr& stream, Packet packet) override;

private:
    void accept();
    std::shared_ptr<UdpStream> getStream(const asio::ip::udp::endpoint& endpoint);

    Server& server;
    asio::io_service& service;
    asio::ip::udp::socket socket;
    asio::ip::udp::endpoint endpoint;
    msgpack::unpacker unp;

    std::unordered_map<asio::ip::udp::endpoint, std::shared_ptr<UdpStream>> streams;
};
} // namespace Scissio::Network
