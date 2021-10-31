#pragma once

#include "NetworkAsio.hpp"
#include "NetworkStream.hpp"
#include <memory>

namespace Scissio::Network {
class Acceptor;

class SCISSIO_API TcpStream : public Stream, public std::enable_shared_from_this<TcpStream> {
public:
    TcpStream(Acceptor& acceptor, asio::ip::tcp::socket socket);
    virtual ~TcpStream() = default;

    void sendRaw(const Packet& packet) override;
    void disconnect() override;
    void receive();

private:
    Acceptor& acceptor;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::endpoint endpoint;
    msgpack::unpacker unp;
};
} // namespace Scissio::Network
