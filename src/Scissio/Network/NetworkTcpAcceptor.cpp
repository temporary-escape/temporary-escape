#include "NetworkTcpAcceptor.hpp"
#include "../Utils/Log.hpp"
#include "NetworkServer.hpp"
#include "NetworkTcpStream.hpp"

#define CMP "NetworkTcpAcceptor"

using namespace Scissio;

Network::TcpAcceptor::TcpAcceptor(Server& server, asio::io_service& service, const int port)
    : server(server), service(service), acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port)),
      socket(service), endpoint(acceptor.local_endpoint()) {
}

Network::TcpAcceptor::~TcpAcceptor() {
    close();
}

void Network::TcpAcceptor::close() {
    std::lock_guard<std::mutex> lock{mutex};
    for (auto& stream : streams) {
        stream->disconnect();
    }
}

void Network::TcpAcceptor::start() {
    accept();
    Log::i(CMP, "Network TCP acceptor started on: [{}]:{}", endpoint.address().to_string(), endpoint.port());
}

void Network::TcpAcceptor::accept() {
    auto self = shared_from_this();
    acceptor.async_accept(socket, [self](const std::error_code ec) {
        if (!ec) {
            if (!self->acceptor.is_open()) {
                return;
            }

            try {
                const auto peer = std::make_shared<TcpStream>(*self, std::move(self->socket));
                self->server.eventConnect(peer);
                peer->receive();

                std::lock_guard<std::mutex> lock{self->mutex};
                self->streams.push_back(peer);
            } catch (std::exception& e) {
                Log::e(CMP, "Network TCP acceptor async_accept error: {}", e.what());
            }
        }

        self->accept();
    });
}

void Network::TcpAcceptor::eventPacket(const StreamPtr& stream, Packet packet) {
    const auto packetId = packet.id;

    try {
        server.eventPacket(stream, std::move(packet));
    } catch (std::exception& e) {
        Log::e(CMP, "Network TCP acceptor failed to accept packet id: {}", packetId);
        backtrace(e);
    }
}

void Network::TcpAcceptor::eventDisconnect(const StreamPtr& stream) {
    try {
        server.eventDisconnect(stream);
    } catch (std::exception& e) {
        Log::e(CMP, "Network TCP acceptor failed to disconnect stream");
        backtrace(e);
    }
}
