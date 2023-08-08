#include "network_tcp_server.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkTcpServer::NetworkTcpServer(asio::io_service& service, NetworkSslContext& ssl, NetworkDispatcher& dispatcher,
                                   const uint32_t port, const bool ipv6) :
    service{service}, strand{service}, ssl{ssl}, dispatcher{dispatcher}, acceptor{service, endpoint(port, ipv6)} {

    logger.info("Starting listening on endpoint: {}", acceptor.local_endpoint());
    accept();
}

NetworkTcpServer::~NetworkTcpServer() {
    stop();
}

void NetworkTcpServer::stop() {
    decltype(peers) temp;
    {
        std::lock_guard<std::mutex> lock{mutex};
        temp = peers;
        peers.clear();
    }
    
    for (const auto& peer : temp) {
        peer->close();
    }

    if (acceptor.is_open()) {
        logger.info("Stopping listening on endpoint: {}", acceptor.local_endpoint());
        acceptor.close();
        logger.info("Server stopped");
    }
}

void NetworkTcpServer::disconnect(const std::shared_ptr<NetworkTcpPeer>& peer) {
    std::lock_guard<std::mutex> lock{mutex};
    peers.erase(std::remove(peers.begin(), peers.end(), peer), peers.end());
}

asio::ip::tcp::endpoint NetworkTcpServer::endpoint(const uint32_t port, const bool ipv6) {
    return {ipv6 ? asio::ip::tcp::v6() : asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(port)};
}

void NetworkTcpServer::accept() {
    auto socket = std::make_shared<Socket>(service, ssl.get());

    acceptor.async_accept(socket->lowest_layer(), strand.wrap([this, socket](const std::error_code ec) {
        if (ec) {
            logger.error("Failed to accept new connection, error: {}", ec.message());
        } else if (acceptor.is_open()) {
            logger.info("Accepting new connection from: {}", socket->lowest_layer().remote_endpoint());
            auto peer = std::make_shared<NetworkTcpPeer>(service, *this, std::move(*socket), dispatcher);
            peer->handshake();
            {
                std::lock_guard<std::mutex> lock{mutex};
                peers.push_back(peer);
            }
            accept();
        }
    }));
}
