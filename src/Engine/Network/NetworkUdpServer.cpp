#include "NetworkUdpServer.hpp"
#include "../Utils/Random.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpServer::NetworkUdpServer(const Config& config, asio::io_service& service, NetworkDispatcher2& dispatcher) :
    config{config},
    service{service},
    dispatcher{dispatcher},
    strand{service},
    socket{
        service,
        asio::ip::udp::endpoint{asio::ip::address::from_string(config.network.serverBindAddress),
                                static_cast<asio::ip::port_type>(config.network.serverPort)},
    },
    stun{config, service, strand, socket},
    localEndpoint{socket.local_endpoint()} {
}

void NetworkUdpServer::start() {
    receive();
    logger.info("UDP server started on address: {}", localEndpoint);
}

NetworkUdpServer::~NetworkUdpServer() {
    logger.info("UDP server destroyed");
}

void NetworkUdpServer::notifyClientConnection(const std::string& address, const uint16_t port,
                                              NotifyCallback callback) {
    logger.info("UDP server sending hello to client address: {} port: {}", address, port);
    static const std::string_view notifyMsg = "IGNOREME";
    auto buff = asio::buffer(notifyMsg.data(), notifyMsg.size());
    const auto endpoint = asio::ip::udp::endpoint{asio::ip::address::from_string(address), port};
    socket.async_send_to(
        buff, endpoint, strand.wrap([c = std::move(callback)](const asio::error_code ec, const size_t received) {
            if (ec) {
                logger.error("UDP server failed to notify client error: {}", ec.message());
            } else {
                c();
            }
        }));
}

void NetworkUdpServer::receive() {
    auto packet = allocatePacket();
    packet->length = maxPacketSize;

    auto buff = asio::buffer(packet->data(), packet->size());

    socket.async_receive_from(
        buff, peerEndpoint, strand.wrap([this, packet](asio::error_code ec, const size_t received) {
            if (ec) {
                // Cancelled?
                if (ec != asio::error::operation_aborted) {
                    logger.error("UDP server receive error: {}", ec.message());
                }
                (void)socket.close(ec);
            } else {
                packet->length = received;

                if (stun.isRunning() && stun.isValid(packet->data(), packet->size())) {
                    stun.parse(packet->data(), packet->size());
                } else {
                    std::lock_guard lock{mutex};
                    auto found = peers.find(peerEndpoint);
                    if (found == peers.end() && peers.size() < 256) {
                        const auto address = socket.local_endpoint().address().to_string();
                        auto peer = std::make_shared<NetworkUdpPeer>(service, dispatcher, socket, peerEndpoint);
                        found = peers.emplace(peerEndpoint, std::move(peer)).first;
                        found->second->sendPublicKey();
                    }

                    if (found != peers.end()) {
                        // service.post([p = found->second, buffer]() { p->onReceive(buffer); });
                        found->second->onReceivePeer(packet);
                    }
                }

                this->receive();
            }
        }));
}

PacketBytesPtr NetworkUdpServer::allocatePacket() {
    std::lock_guard<std::mutex> lock{packetPoolMutex};
    const auto ptr = packetPool.allocate();
    new (ptr) PacketBytes();
    return PacketBytesPtr{
        ptr,
        [this, pool = &packetPool](PacketBytes* p) {
            p->~PacketBytes();
            std::lock_guard<std::mutex> lock{packetPoolMutex};
            pool->deallocate(p);
        },
    };
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

    logger.warn("NetworkUdpServer stop done");
}

/*asio::ip::udp::endpoint NetworkUdpServer::endpoint(uint32_t port, bool ipv6) {
    return asio::ip::udp::endpoint();
}*/
