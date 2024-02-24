#include "NetworkUdpClient.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Random.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpClient::NetworkUdpClient(const Config& config, asio::io_service& service, NetworkDispatcher2& dispatcher) :
    NetworkUdpStream{service, true},
    dispatcher{dispatcher},
    socket{
        service,
        asio::ip::udp::endpoint{
            asio::ip::address::from_string(config.network.clientBindAddress),
            0,
        },
    },
    stun{config, service, strand, socket},
    localEndpoint{socket.local_endpoint()} {
}

void NetworkUdpClient::start() {
    receive();
    logger.info("UDP client started on address: {}", socket.local_endpoint());
}

NetworkUdpClient::~NetworkUdpClient() {
    logger.info("UDP client destroyed");
}

void NetworkUdpClient::stop() {
    sendClosePacket();
    stopInternal();
}

void NetworkUdpClient::close() {
    stop();
}

void NetworkUdpClient::stopInternal() {
    auto self = shared_from_this();
    forceClosed();
    strand.post([self]() {
        asio::error_code ec;
        (void)self->socket.close(ec);
        if (ec) {
            logger.error("UDP client close error: {}", ec.message());
        }
    });
}

void NetworkUdpClient::sendPacket(const PacketBytesPtr& packet) {
    auto buff = asio::buffer(packet->data(), packet->size());
    auto self = shared_from_this();
    socket.async_send_to(buff, endpoint, strand.wrap([self, packet](asio::error_code ec, const size_t sent) {
        if (ec) {
            logger.error("UDP client send error: {}", ec.message());
            self->stopInternal();
        } else {
            self->onPacketSent(packet);
        }
    }));
}

void NetworkUdpClient::receive() {
    auto packet = allocatePacket();
    packet->length = maxPacketSize;

    auto buff = asio::buffer(packet->data(), packet->size());

    auto self = shared_from_this();
    socket.async_receive_from(
        buff, peerEndpoint, strand.wrap([self, packet](asio::error_code ec, const size_t received) {
            if (ec) {
                // Cancelled?
                if (ec != asio::error::operation_aborted) {
                    logger.error("UDP client receive error: {}", ec.message());
                }
                self->stopInternal();
            } else {
                packet->length = received;

                if (self->stun.isRunning() && self->stun.isValid(packet->data(), packet->size())) {
                    self->stun.parse(packet->data(), packet->size());
                } else if (self->peerEndpoint == self->endpoint) {
                    // logger.info("UDP client received {} bytes", packet->size());
                    self->onReceive(packet);
                }

                self->receive();
            }
        }));
}

void NetworkUdpClient::connect(const std::string& address, const uint16_t port) {
    {
        std::lock_guard lock{connectedLock};
        connected = false;
    }

    logger.info("UDP client connecting to address: {} port: {}", address, port);

    const asio::ip::udp::resolver::query query(address, std::to_string(port));
    asio::ip::udp::resolver resolver{service};

    auto future = resolver.async_resolve(query, asio::use_future);
    if (future.wait_for(std::chrono::seconds{1}) != std::future_status::ready) {
        EXCEPTION("Failed to resolve address: {}", address);
    }

    // Find the endpoint that matches our protocol
    auto found = false;
    for (const auto& e : future.get()) {
        // Skip IPv6 endpoint if our socket is IPv4
        if (localEndpoint.protocol() == asio::ip::udp::v4() && e.endpoint().protocol() != asio::ip::udp::v4()) {
            continue;
        }

        endpoint = e.endpoint();
        found = true;
        break;
    }

    if (!found) {
        EXCEPTION("Failed to resolve server address: {}", address);
    }

    this->address = fmt::format("{}", endpoint);

    const auto& publicKey = getPublicKey();
    const auto buff = asio::buffer(publicKey.data(), publicKey.size());

    auto self = shared_from_this();
    socket.async_send_to(buff, endpoint, [self](const asio::error_code ec, const size_t sent) {
        (void)self;
        if (ec) {
            logger.error("Failed to sent public key to the server error: {}", ec.message());
        } else {
            logger.info("UDP client sent public key to remote: {}", self->endpoint);
        }
    });

    std::unique_lock lock{connectedLock};
    if (!connectedCv.wait_for(lock, std::chrono::seconds{1}, [this] { return connected; })) {
        EXCEPTION("Failed to connect to address: {}", address);
    }
}

void NetworkUdpClient::onConnected() {
    logger.info("UDP client connected remote: {}", endpoint);

    {
        std::lock_guard lock{connectedLock};
        connected = true;
    }
    connectedCv.notify_one();

    auto self = shared_from_this();
    strand.post([self]() { self->dispatcher.onAcceptSuccess(self); });
}

void NetworkUdpClient::onDisconnected() {
    logger.warn("UDP client disconnected remote: {}", endpoint);

    auto self = shared_from_this();
    strand.post([self]() { self->dispatcher.onDisconnect(self); });
}

std::shared_ptr<NetworkUdpStream> NetworkUdpClient::makeShared() {
    return shared_from_this();
}

void NetworkUdpClient::onObjectReceived(msgpack::object_handle oh) {
    if (!isEstablished()) {
        return;
    }

    auto o = std::make_shared<decltype(oh)>(std::move(oh));
    auto self = shared_from_this();
    strand.post([self, o]() { self->dispatcher.onObjectReceived(self, o); });
}
