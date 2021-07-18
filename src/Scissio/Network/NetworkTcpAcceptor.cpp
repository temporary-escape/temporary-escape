#include "NetworkTcpAcceptor.hpp"
#include "../Utils/Log.hpp"
#include "NetworkServer.hpp"
#include "NetworkTcpStream.hpp"

using namespace Scissio;

Network::TcpAcceptor::TcpAcceptor(Server& server, asio::io_service& service, const int port)
    : server(server), service(service), acceptor(service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port)),
      socket(service), endpoint(acceptor.local_endpoint()) {
}

Network::TcpAcceptor::~TcpAcceptor() {
    close();
}

void Network::TcpAcceptor::close() {
    for (auto& stream : streams) {
        stream->disconnect();
    }
}

void Network::TcpAcceptor::start() {
    accept();
    Log::i("Network TCP acceptor started on: {}", endpoint.address().to_string());
}

void Network::TcpAcceptor::accept() {
    auto self = shared_from_this();
    acceptor.async_accept(socket, [self](const std::error_code ec) {
        if (!ec) {
            if (!self->acceptor.is_open()) {
                return;
            }

            try {
                // auto endpoint = socket.remote_endpoint();
                const auto peer = std::make_shared<TcpStream>(*self, std::move(self->socket));
                peer->receive();
                self->streams.push_back(peer);
                // onAccept(peer);
                // peer->receive();
                // acceptPeer(peer);
            } catch (std::exception& e) {
                Log::e("Network TCP acceptor async_accept error: {}", e.what());
            }
        }

        self->accept();
    });
}

void Network::TcpAcceptor::receive(const StreamPtr& stream, Packet packet) {
    const auto packetId = packet.id;
    const auto sessionId = packet.sessionId;

    try {
        server.receive(stream, std::move(packet));
    } catch (std::exception& e) {
        Log::e("Failed to accept packet id: {} session: {}", packetId, sessionId);
        backtrace(e);
    }
}
