#include "NetworkUdpServer.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpServer::NetworkUdpServer(const Config& config, asio::io_service& service) :
    config{config},
    service{service},
    strand{service},
    socket{
        service,
        asio::ip::udp::endpoint{asio::ip::address::from_string(config.network.serverBindAddress),
                                static_cast<asio::ip::port_type>(config.network.serverPort)},
    },
    stun{config, service, strand, socket},
    localEndpoint{socket.local_endpoint()} {

    receive();
    logger.info("UDP server started on address: {}", localEndpoint);

    // Send STUN request only if we are not running on localhost
    if (config.network.serverBindAddress != "::1" && config.network.serverBindAddress != "127.0.0.1") {
        stun.sendRequest();
    }
}

NetworkUdpServer::~NetworkUdpServer() {
    logger.info("UDP server destroyed");
}

void NetworkUdpServer::receive() {
    auto buff = asio::buffer(buffer.data(), buffer.size());
    socket.async_receive_from(buff, peerEndpoint, strand.wrap([this](const asio::error_code ec, const size_t received) {
        if (ec) {
            logger.error("UDP server receive error: {}", ec.message());
            socket.close();
        } else {
            logger.error("UDP server receive endpoint: {}", peerEndpoint);

            if (stun.isRunning() && stun.isValid(buffer.data(), received)) {
                stun.parse(buffer.data(), received);
                if (stun.hasResult()) {
                    logger.info("UDP server public mapped address: {}", stun.getResult());
                }
            } else {
                std::lock_guard lock{mutex};
                auto found = peers.find(peerEndpoint);
                if (found == peers.end() && peers.size() < 256) {
                    const auto address = socket.local_endpoint().address().to_string();
                    auto peer = std::make_shared<NetworkUdpPeer>(service, socket, peerEndpoint);
                    found = peers.emplace(peerEndpoint, std::move(peer)).first;
                    found->second->sendHello();
                }
            }

            this->receive();
        }
    }));
}

void NetworkUdpServer::stop() {
    std::lock_guard lock{mutex};
    for (auto& [_, peer] : peers) {
        peer->close();
    }

    strand.post([this]() {
        asio::error_code ec;
        (void)socket.close(ec);
        if (ec) {
            logger.error("UDP server close error: {}", ec.message());
        }
    });
}

/*asio::ip::udp::endpoint NetworkUdpServer::endpoint(uint32_t port, bool ipv6) {
    return asio::ip::udp::endpoint();
}*/
