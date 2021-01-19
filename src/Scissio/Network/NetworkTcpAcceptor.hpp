#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"

namespace Scissio::Network {
class Server;
class TcpStream;

class SCISSIO_API TcpAcceptor : public Acceptor, public std::enable_shared_from_this<TcpAcceptor> {
public:
    explicit TcpAcceptor(Server& server, asio::io_service& service, int port);
    virtual ~TcpAcceptor();

    void close();
    void start() override;
    void receive(const StreamPtr& stream, const Packet& packet) override;

private:
    void accept();

    Server& server;
    asio::io_service& service;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::endpoint endpoint;

    std::vector<std::shared_ptr<TcpStream>> streams;
};
} // namespace Scissio::Network
