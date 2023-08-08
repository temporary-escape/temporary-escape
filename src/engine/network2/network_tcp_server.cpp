#include "network_tcp_server.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkTcpServer::NetworkTcpServer(asio::io_service& service, NetworkSslContext& ssl, NetworkDispatcher& dispatcher,
                                   const uint32_t port, const bool ipv6) :
    service{service}, ssl{ssl}, dispatcher{dispatcher}, acceptor{service, endpoint(port, ipv6)} {

    logger.info("Starting listening on endpoint: {}", acceptor.local_endpoint());
    accept();
}

NetworkTcpServer::~NetworkTcpServer() {
    stop();
}

void NetworkTcpServer::stop() {
    for (const auto& peer : peers) {
        peer->close();
    }
    peers.clear();

    if (acceptor.is_open()) {
        logger.info("Stopping listening on endpoint: {}", acceptor.local_endpoint());
        acceptor.close();
        logger.info("Server stopped");
    }
}

asio::ip::tcp::endpoint NetworkTcpServer::endpoint(const uint32_t port, const bool ipv6) {
    return {ipv6 ? asio::ip::tcp::v6() : asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(port)};
}

void NetworkTcpServer::accept() {
    auto socket = std::make_shared<Socket>(service, ssl.get());

    acceptor.async_accept(socket->lowest_layer(), [this, socket](const std::error_code ec) {
        if (ec) {
            logger.error("Failed to accept new connection, error: {}", ec.message());
        } else if (acceptor.is_open()) {
            logger.info("Accepting new connection from: {}", socket->lowest_layer().remote_endpoint());
            auto peer = std::make_shared<NetworkTcpPeer>(service, std::move(*socket), dispatcher);
            peer->handshake();
            peers.push_back(peer);
            accept();
        }
    });
}
