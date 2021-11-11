#pragma once

#include "NetworkAcceptor.hpp"
#include "NetworkAsio.hpp"

namespace Scissio::Network {
class Server;
class TcpStream;

class SCISSIO_API TcpAcceptor : public Acceptor, public std::enable_shared_from_this<TcpAcceptor> {
public:
    explicit TcpAcceptor(EventListener& listener, Crypto::Ecdhe& ecdhe, asio::io_service& service, int port);
    virtual ~TcpAcceptor();

    void close();
    void start() override;

private:
    void accept();

    Crypto::Ecdhe& ecdhe;

    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::endpoint endpoint;

    std::mutex mutex;
    std::vector<std::shared_ptr<TcpStream>> streams;
};
} // namespace Scissio::Network
