#pragma once

#include "NetworkTcpStream.hpp"

namespace Engine {
template <typename Handler, typename Sink> class ENGINE_API NetworkTcpClient;

template <typename Handler, typename Sink> class ENGINE_API NetworkTcpConnector : public NetworkTcpStream<Sink> {
public:
    explicit NetworkTcpConnector(NetworkTcpClient<Handler, Sink>& client, Crypto::Ecdhe& ecdhe,
                                 asio::ip::tcp::socket socket)
        : NetworkTcpStream<Sink>(ecdhe, std::move(socket)), client(client) {
    }
    virtual ~NetworkTcpConnector() = default;

private:
    void onConnected() override;
    void onReceive(Packet packet) override;

    NetworkTcpClient<Handler, Sink>& client;
};
} // namespace Engine
