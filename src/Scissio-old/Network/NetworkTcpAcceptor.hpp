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
    void eventPacket(const StreamPtr& stream, Packet packet) override;
    void eventDisconnect(const StreamPtr& stream) override;

private:
    void accept();

    Server& server;
    asio::io_service& service;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::endpoint endpoint;

    std::mutex mutex;
    std::vector<std::shared_ptr<TcpStream>> streams;
};
} // namespace Scissio::Network
