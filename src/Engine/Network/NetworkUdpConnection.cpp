#include "NetworkUdpConnection.hpp"
#include "../Utils/Log.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

NetworkUdpConnection::Writer::Writer(NetworkUdpConnection& conn, const PacketType type) :
    Packer{*this}, conn{conn}, type{type}, lock{conn.writeMutex} {
}

void NetworkUdpConnection::Writer::write(const char* data, size_t length) {
    while (length > 0) {
        const auto toWrite = std::min(temp.size() - written, length);
        std::memcpy(&temp[written], data, toWrite);
        length -= toWrite;
        written += toWrite;

        if (written == temp.size()) {
            flush();
        }
    }
}

void NetworkUdpConnection::Writer::flush() {
    if (!conn.aes) {
        return;
    }

    logger.debug("UDP connection flushing: {} bytes", written);
    if (written > maxPacketDataSize) {
        EXCEPTION("written > maxPacketDataSize");
    }

    auto packet = conn.allocatePacket();
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    header.type = type;
    header.sequence = 0;
    packet->length = sizeof(PacketHeader);

    packet->length += conn.aes->encrypt(temp.data(), packet->data() + packet->length, written);

    written = 0;

    if (type == PacketType::DataReliable) {
        conn.enqueuePacket(packet);
    } else {
        conn.sendPacket(packet);
    }
}

NetworkUdpConnection::NetworkUdpConnection(asio::io_service& service) :
    service{service}, strand{service}, publicKey{ecdh.getPublicKey()}, enqueuePos{sendQueueList.end()} {
}

/*void NetworkUdpConnection::releasePacket(PacketData* packet) {
    memoryPool.deallocate(packet);
}*/

/*void NetworkUdpConnection::onSentPacket(PacketData* packet, const bool error) {
    releasePacket(packet);
}*/

void NetworkUdpConnection::onReceive(const PacketBytesPtr& packet) {
    // Is it public key?
    if (ECDH::isPublicKey({reinterpret_cast<const char*>(packet->data()), packet->size()})) {
        if (sharedSecret.empty()) {
            logger.info("UDP connection received public key from the server");
            sharedSecret = ecdh.deriveSharedSecret({reinterpret_cast<const char*>(packet->data()), packet->size()});
            logger.debug("UDP connection shared secret computed: {}",
                         toHexString(sharedSecret.data(), sharedSecret.size()));
            aes = std::make_unique<AES>(sharedSecret);

            onConnected();
        }
        return;
    } else if (aes && packet->size() >= sizeof(PacketHeader) &&
               AES::getDecryptSize(packet->size() - sizeof(PacketHeader)) <= plaintext.size()) {

        const auto& header = *reinterpret_cast<const PacketHeader*>(packet->data());

        if (header.type == PacketType::Ack) {
            onAckReceived(packet);
        } else if (header.type == PacketType::DataReliable) {
            sendAck(packet);

            /*if (header.sequence != receiveNum) {
                return;
            }*/
            /*const auto index = header.sequence % receiveQueue.size();
            if (!receiveQueue[index]) {
                receiveQueue[index] =
            }*/
        }

        /*const auto* encSrc = reinterpret_cast<const uint8_t*>(data) + sizeof(PacketHeader);
        const auto encLength = length - sizeof(PacketHeader);
        if (encLength > 0) {
            const auto read = aes->decrypt(encSrc, plaintext.data(), encLength);
            logger.info("UDP connection decrypted: {}", read);
        }*/
        //}
    }
}

void NetworkUdpConnection::sendAck(const PacketBytesPtr& packet) {
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    header.type = PacketType::Ack;
    packet->length = sizeof(PacketHeader);
    sendPacket(packet);
}

void NetworkUdpConnection::onAckReceived(const PacketBytesPtr& packet) {
    const auto& header = *reinterpret_cast<const PacketHeader*>(packet->data());
    logger.info("UDP connection got ack: {}", header.sequence);

    // Out of the window?
    if (header.sequence < ackNum || header.sequence >= ackNum + packetWindowSize) {
        return;
    }

    auto index = header.sequence % packetQueueSize;
    auto it = sendQueueList.begin();
    if (index < ackNum % packetQueueSize) {
        ++it;
    }

    logger.info("Dequeue packet index: {}", index);
    auto& ptr = it->at(index);
    if (!ptr) {
        return;
    }

    // Mark packet as done
    ptr.reset();

    // Move the ack position forward
    if (header.sequence == ackNum) {
        while (!it->at(index) && it != sendQueueList.end()) {
            index++;
            ackNum++;

            if (index == packetQueueSize) {
                it++;
                index = 0;
                sendQueueList.pop_front();
            }
        }

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

PacketBytesPtr NetworkUdpConnection::allocatePacket() {
    std::lock_guard<std::mutex> lock{packetPoolMutex};
    const auto ptr = packetPool.allocate();
    new (ptr) PacketBytes();
    return PacketBytesPtr{
        ptr,
        [pool = &packetPool](PacketBytes* p) {
            p->~PacketBytes();
            pool->deallocate(p);
        },
    };
}

void NetworkUdpConnection::enqueuePacket(const PacketBytesPtr& packet) {
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
        logger.info("Enqueue packet: {} index: {}", header.sequence, index);
        self->enqueuePos->at(index) = packet;

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

void NetworkUdpConnection::onPacketSent(const PacketBytesPtr& packet) {
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    if (header.type == PacketType::DataReliable) {
        logger.info("Sent packet: {} sendNum: {}", header.sequence, sendNum);
        // assert(header.sequence == sendNum);

        sending = false;
        // ++sendNum;
        startSendQueue();
    }
}

void NetworkUdpConnection::startSendQueue() {
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
    if (!packet) {
        return;
    }

    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    logger.info("UDP client Sending packet: {}", header.sequence);

    sending = true;
    ++sendNum;

    sendPacket(packet);
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
