#pragma once

#include "NetworkTcpStream.hpp"

namespace Engine {
template <typename Handler, typename Sink> class ENGINE_API NetworkTcpServer;

template <typename Handler, typename Sink> class ENGINE_API NetworkTcpPeer : public NetworkTcpStream<Sink> {
public:
    explicit NetworkTcpPeer(NetworkTcpServer<Handler, Sink>& server, Crypto::Ecdhe& ecdhe, asio::ip::tcp::socket socket)
        : NetworkTcpStream<Sink>(ecdhe, std::move(socket)), server(server) {
    }

private:
    void onConnected() override;
    void onReceive(Packet packet) override;

    NetworkTcpServer<Handler, Sink>& server;
};
} // namespace Engine
