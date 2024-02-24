#include "NetworkUdpPeer.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpPeer::NetworkUdpPeer(asio::io_service& service, NetworkDispatcher2& dispatcher, asio::ip::udp::socket& socket,
                               asio::ip::udp::endpoint endpoint) :
    NetworkUdpStream{service, false},
    service{service},
    dispatcher{dispatcher},
    socket{socket},
    endpoint{endpoint},
    address{fmt::format("{}", endpoint)} {

    logger.info("UDP peer created local: {} remote: {}", socket.local_endpoint(), endpoint);
}

NetworkUdpPeer::~NetworkUdpPeer() {
    logger.info("UDP peer destroyed remote: {}", endpoint);
}

void NetworkUdpPeer::close() {
    sendClosePacket();
    forceClosed();
}

void NetworkUdpPeer::sendPublicKey() {
    auto self = shared_from_this();
    const auto& publicKey = getPublicKey();
    auto buff = asio::buffer(publicKey.data(), publicKey.size());
    socket.async_send_to(buff, endpoint, strand.wrap([self](const asio::error_code ec, const size_t sent) {
        if (ec) {
            logger.error("UDP peer failed to send to remote: {} error: {}", self->endpoint, ec.message());
            self->forceClosed();
        } else {
            logger.info("UDP peer sent public key to remote: {}", self->endpoint);
        }
    }));
}

void NetworkUdpPeer::sendPacket(const PacketBytesPtr& packet) {
    auto self = shared_from_this();
    auto buff = asio::buffer(packet->data(), packet->size());
    socket.async_send_to(buff, endpoint, strand.wrap([self, packet](asio::error_code ec, const size_t sent) {
        if (ec) {
            logger.error("UDP peer send error: {}", ec.message());
            self->forceClosed();
        } else {
            self->onPacketSent(packet);
        }
    }));
}

void NetworkUdpPeer::onReceivePeer(const PacketBytesPtr& packet) {
    auto self = shared_from_this();
    strand.post([self, packet]() {
        // logger.info("UDP peer received {} bytes", packet->size());
        self->onReceive(packet);
    });
}

void NetworkUdpPeer::onConnected() {
    logger.info("UDP peer connected remote: {}", endpoint);

    auto self = shared_from_this();
    strand.post([self]() { self->dispatcher.onAcceptSuccess(self); });
}

void NetworkUdpPeer::onDisconnected() {
    logger.warn("UDP peer disconnected remote: {}", endpoint);

    auto self = shared_from_this();
    strand.post([self]() { self->dispatcher.onDisconnect(self); });
}

std::shared_ptr<NetworkUdpStream> NetworkUdpPeer::makeShared() {
    return shared_from_this();
}

void NetworkUdpPeer::onObjectReceived(msgpack::object_handle oh) {
    if (!isEstablished()) {
        return;
    }

    auto o = std::make_shared<decltype(oh)>(std::move(oh));
    auto self = shared_from_this();
    strand.post([self, o]() { self->dispatcher.onObjectReceived(self, o); });
}
