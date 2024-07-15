#include "NetworkStream.hpp"
#include "../Utils/Exceptions.hpp"

using namespace Engine;

NetworkStream::Writer::Writer(NetworkStream& stream, const PacketType type) :
    Packer{*this}, stream{stream}, type{type}, lock{stream.mutex} {
}

void NetworkStream::Writer::write(const char* data, size_t length) {
    while (length > 0) {
        const auto toWrite = std::min(temp.size() - written, length);
        std::memcpy(&temp[written], data, toWrite);
        length -= toWrite;
        written += toWrite;
        data += toWrite;

        if (written == temp.size()) {
            flush();
        }
    }
}

void NetworkStream::Writer::flush() {
    if (written == 0) {
        return;
    }

    if (!stream.aes) {
        EXCEPTION("NetworkStream has not been initialized");
    }

    // logger.debug("UDP connection flushing: {} bytes", written);
    if (written > maxPacketDataSize) {
        EXCEPTION("written > maxPacketDataSize");
    }

    auto packet = stream.allocatePacket();
    auto& header = *reinterpret_cast<PacketHeader*>(packet->data());
    header.type = type;
    header.sequence = 0;
    packet->length = sizeof(PacketHeader);

    auto* messageData = packet->data() + packet->length;
    packet->length += stream.aes->encrypt(temp.data(), messageData, written);
    packet->length += stream.hmac->sign(messageData, packet->data() + packet->length, written + AES::ivecLength);

    written = 0;

    stream.enqueuePacket(packet);
}

void NetworkStream::onSharedSecret(const std::vector<uint8_t>& sharedSecret) {
    aes = std::make_unique<AES>(sharedSecret);
    hmac = std::make_unique<HMAC>(sharedSecret);
}

size_t NetworkStream::decrypt(const void* src, void* dst, const size_t size, bool& verify) {
    if (!aes || !hmac || size <= HMAC::resultSize) {
        return 0;
    }
    const auto length = aes->decrypt(src, dst, size - HMAC::resultSize);

    hmac->sign(src, verifyBuffer.data(), size - HMAC::resultSize);

    const auto signatureSrc = reinterpret_cast<const uint8_t*>(src) + size - HMAC::resultSize;
    verify = std::memcmp(verifyBuffer.data(), signatureSrc, HMAC::resultSize) == 0;

    return length;
}
