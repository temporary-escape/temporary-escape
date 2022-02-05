#pragma once

#include "NetworkAsio.hpp"
#include "NetworkStream.hpp"
#include <memory>

namespace Engine::Network {
class Acceptor;

class ENGINE_API TcpStream : public Stream, public std::enable_shared_from_this<TcpStream> {
public:
    TcpStream(Acceptor& acceptor, Crypto::Ecdhe& ecdhe, asio::ip::tcp::socket socket);
    virtual ~TcpStream();

    void sendRaw(const Packet& packet) override;
    void disconnect();
    void receive();

private:
    Acceptor& acceptor;
    std::atomic_bool flag;
    asio::ip::tcp::socket socket;
    asio::ip::tcp::endpoint endpoint;
    msgpack::unpacker unp;
};
} // namespace Engine::Network
