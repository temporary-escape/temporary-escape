#include "NetworkUdpStream.hpp"
#include "../Utils/Log.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static auto ackTimerInterval = std::chrono::milliseconds{10};
static auto ackTimerResentInterval = 25;
static auto ackTimerDeadline = 100;

NetworkUdpStream::NetworkUdpStream(asio::io_service& service) :
    service{service},
    strand{service},
    publicKey{ecdh.getPublicKey()},
    enqueuePos{sendQueueList.end()},
    ackTimer{service, ackTimerInterval} {
}

void NetworkUdpStream::startAckTimer() {
    auto self = makeShared();
    ackTimer.expires_at(ackTimer.expires_at() + ackTimerInterval);
    ackTimer.async_wait(strand.wrap([self](const asio::error_code ec) {
        if (ec) {
            // Cancelled?
            if (ec == asio::error::operation_aborted) {
                return;
            }
            logger.error("UDP connection ack timer failed error: {}", ec.message());
        } else {
            self->processQueue();
        }
    }));
}

void NetworkUdpStream::stopAckTimer() {
    asio::error_code ec;
    (void)ackTimer.cancel(ec);
}

/*void NetworkUdpConnection::releasePacket(PacketData* packet) {
    memoryPool.deallocate(packet);
}*/

/*void NetworkUdpConnection::onSentPacket(PacketData* packet, const bool error) {
    releasePacket(packet);
}*/

void NetworkUdpStream::onReceive(const PacketBytesPtr& packet) {
    // Is it public key?
    if (ECDH::isPublicKey({reinterpret_cast<const char*>(packet->data()), packet->size()})) {
        if (sharedSecret.empty()) {
            logger.info("UDP connection received public key from the server");
            sharedSecret = ecdh.deriveSharedSecret({reinterpret_cast<const char*>(packet->data()), packet->size()});
            // logger.debug("UDP connection shared secret computed: {}",
            //              toHexString(sharedSecret.data(), sharedSecret.size()));
            onSharedSecret(sharedSecret);

            onConnected();
        }
        return;
    } else if (!sharedSecret.empty() && packet->size() >= sizeof(PacketHeader) &&
               AES::getDecryptSize(packet->size() - sizeof(PacketHeader)) <= maxPacketDataSize) {

        const auto& header = *reinterpret_cast<const PacketHeader*>(packet->data());

        if (header.type == PacketType::Ack) {
            onAckReceived(packet);
        } else if (header.type == PacketType::DataReliable) {
            receivePacketReliable(packet);
            sendAck(packet);
        } else if (header.type == PacketType::Data) {
            receivePacketUnreliable(packet);
        }
    }
}

void NetworkUdpStream::receivePacketUnreliable(const PacketBytesPtr& packet) {
    const auto length =
        decrypt(packet->data() + sizeof(PacketHeader), plaintext.data(), packet->size() - sizeof(PacketHeader));

    auto oh = msgpack::unpack(reinterpret_cast<const char*>(plaintext.data()), length);
    receiveObject(std::move(oh));
}

void NetworkUdpStream::receivePacketReliable(const PacketBytesPtr& packet) {
    const auto& header = *reinterpret_cast<const PacketHeader*>(packet->data());
    // logger.info("Received: {}", header.sequence);

    if (header.sequence < receiveNum || header.sequence >= receiveNum + packetQueueSize) {
        // Out of bounds
        return;
    }

    auto index = header.sequence % packetQueueSize;
    auto& item = receiveQueue.at(index);

    if (item.length) {
        // Do not rewrite received data with a duplicate
        return;
    }

    // Read the packet contents
    item.length =
        decrypt(packet->data() + sizeof(PacketHeader), item.buffer.data(), packet->size() - sizeof(PacketHeader));
    // logger.info("Decrypted: {} bytes", item.length);

    // Process receive queue
    for (uint32_t i = receiveNum; i < receiveNum + packetQueueSize; i++) {
        index = i % packetQueueSize;
        auto& p = receiveQueue.at(index);
        if (p.length) {
            consumePacket(p);
        } else {
            break;
        }

        p.length = 0;
        receiveNum++;
    }
}

void NetworkUdpStream::consumePacket(const ReceiveQueueItem& packet) {
    unp.reserve_buffer(packet.length);
    std::memcpy(unp.buffer(), packet.buffer.data(), packet.length);
    unp.buffer_consumed(packet.length);

    msgpack::object_handle oh{};
    while (unp.next(oh)) {
        // logger.info("Got msgpack object!");
        receiveObject(std::move(oh));
        oh = msgpack::object_handle{};
    }
}

void NetworkUdpStream::receiveObject(msgpack::object_handle oh) {
    if (!Detail::validateMessageObject(oh)) {
        logger.error("Received malformed message");
    } else {
        onObjectReceived(std::move(oh));
    }
}

void NetworkUdpStream::sendAck(const PacketBytesPtr& packet) {
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    header.type = PacketType::Ack;
    packet->length = sizeof(PacketHeader);
    sendPacket(packet);
}

void NetworkUdpStream::onAckReceived(const PacketBytesPtr& packet) {
    const auto& header = *reinterpret_cast<const PacketHeader*>(packet->data());
    // logger.info("UDP connection got ack: {}", header.sequence);

    // Out of the window?
    if (header.sequence < ackNum || header.sequence >= ackNum + packetWindowSize) {
        logger.warn("UDP connection got out of bounds ack: {}", header.sequence);
        return;
    }

    auto index = header.sequence % packetQueueSize;
    auto it = sendQueueList.begin();
    if (index < ackNum % packetQueueSize) {
        ++it;
    }

    // logger.info("Dequeue packet index: {}", index);
    auto& ptr = it->at(index);
    if (!ptr.buffer) {
        logger.warn("UDP connection got duplicate ack: {}", header.sequence);
        return;
    }

    // Mark packet as done
    ptr.buffer.reset();

    // Move the ack position forward
    if (header.sequence == ackNum) {
        // logger.info("Advancing ackNum: {}", ackNum);
        while (!it->at(index).buffer && it != sendQueueList.end() && ackNum < sendNum) {
            index++;
            ackNum++;

            if (index == packetQueueSize) {
                it++;
                index = 0;
                sendQueueList.pop_front();
            }
        }

        // logger.info("Advanced ackNum to: {}", ackNum);
        startSendQueue();
    }

    /*const auto index = header.sequence % sendQueue.size();
    logger.info("UDP connection got ack for: {} index: {}", header.sequence, index);

    auto& ptr = sendQueue[index];
    if (!ptr) {
        return;
    }

    const auto& our = *reinterpret_cast<const PacketHeader*>(ptr->data());
    if (our.sequence == header.sequence) {
        ptr.reset();
    }*/
}

/*void NetworkUdpConnection::sendAck(const PacketHeader& read) {
    auto packet = std::make_shared<PacketBuffer>(sizeof(PacketHeader));
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    header.type = PacketType::Ack;
    header.sequence = read.sequence;
    sendPacket(packet);
}*/

PacketBytesPtr NetworkUdpStream::allocatePacket() {
    std::lock_guard<std::mutex> lock{packetPoolMutex};
    const auto ptr = packetPool.allocate();
    new (ptr) PacketBytes();
    return PacketBytesPtr{
        ptr,
        [self = makeShared(), pool = &packetPool](PacketBytes* p) {
            p->~PacketBytes();
            std::lock_guard<std::mutex> lock{self->packetPoolMutex};
            pool->deallocate(p);
        },
    };
}

void NetworkUdpStream::enqueuePacket(const PacketBytesPtr& packet) {
    if (packet->size() < sizeof(PacketHeader)) {
        EXCEPTION("Packet is too small");
    }

    auto self = makeShared();
    strand.post([self, packet]() {
        auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
        header.sequence = self->sequenceNum++;

        // No queue available or we are at the end
        if (self->enqueuePos == self->sendQueueList.end()) {
            self->sendQueueList.emplace_back();
            self->enqueuePos = self->sendQueueList.end();
            self->enqueuePos--;
        }

        // Remember the packet
        const auto index = header.sequence % packetQueueSize;
        // logger.info("Enqueue packet: {} index: {}", header.sequence, index);
        self->enqueuePos->at(index).buffer = packet;
        self->enqueuePos->at(index).time = 0;

        // Did we fill up the current send queue completely?
        // Jump to the next one...
        if (index + 1 == self->enqueuePos->size()) {
            self->enqueuePos++;
        }

        if (!self->sending) {
            self->startSendQueue();
        }
    });
}

void NetworkUdpStream::onPacketSent(const PacketBytesPtr& packet) {
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    if (header.type == PacketType::DataReliable) {
        // logger.info("Sent packet: {} sendNum: {}", header.sequence, sendNum);
        //  assert(header.sequence == sendNum);

        sending = false;
        // ++sendNum;
        startSendQueue();
    }
}

void NetworkUdpStream::startSendQueue() {
    /*const auto jump = sendNum / packetQueueSize;
    const auto index = sendNum % packetQueueSize;

    auto it = sendQueueList.begin();
    std::advance(it, jump);

    logger.info("UDP client sending jump: {} index: {}", jump, index);*/

    if (sendNum - ackNum >= packetWindowSize) {
        return;
    }

    const auto index = sendNum % packetQueueSize;
    auto it = sendQueueList.begin();
    if (index < ackNum % packetQueueSize) {
        ++it;
    }

    auto& packet = it->at(index);
    if (!packet.buffer) {
        return;
    }

    auto& header = *reinterpret_cast<PacketHeader*>(packet.buffer->data());
    // logger.info("UDP client Sending packet: {}", header.sequence);

    sending = true;
    ++sendNum;

    sendPacket(packet.buffer);
}

void NetworkUdpStream::processQueue() {
    for (uint32_t i = ackNum; i < std::min<uint32_t>(ackNum + packetWindowSize, sendNum); i++) {
        const auto index = i % packetQueueSize;
        auto it = sendQueueList.begin();
        if (index < ackNum % packetQueueSize) {
            ++it;
        }

        auto& packet = it->at(index);
        if (!packet.buffer) {
            continue;
        }

        packet.time += 1;
        if (packet.time == ackTimerDeadline) {
            // Deadline
            logger.error("UDP deadline has been reached on ack: {}", i);
        } else if (packet.time == ackTimerResentInterval) {
            // Resend!
            logger.warn("Resending packet: {}", i);
            sendPacket(packet.buffer);
        }
    }

    startAckTimer();
}

/*NetworkPacketRingBuffer::NetworkPacketRingBuffer() {
    chunks.push_back(std::make_unique<Chunk>());
    head.chunk = chunks.back().get();
    head.it = head.chunk->items.begin();

    tail.chunk = chunks.back().get();
    tail.it = tail.chunk->items.begin();
}

uint8_t* NetworkPacketRingBuffer::allocate() {
    if (head.it == head.chunk->items.end()) {
        chunks.push_back(std::make_unique<Chunk>());
        head.chunk = chunks.back().get();
        head.it = head.chunk->items.begin();
    }

    head.it->used = true;
    auto* res = head.it->buffer.data();

    head.it++;

    return res;
}*/

/*void NetworkUdpConnection::onSent(const uint32_t sequence) {
}*/
