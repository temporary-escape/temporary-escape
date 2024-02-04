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

        if (written == temp.size()) {
            flush();
        }
    }
}

void NetworkStream::Writer::flush() {
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

    packet->length += stream.aes->encrypt(temp.data(), packet->data() + packet->length, written);

    written = 0;

    stream.enqueuePacket(packet);
}

void NetworkStream::onSharedSecret(const std::vector<uint8_t>& sharedSecret) {
    aes = std::make_unique<AES>(sharedSecret);
}

size_t NetworkStream::decrypt(const void* src, void* dst, const size_t size) {
    if (!aes) {
        return 0;
    }
    return aes->decrypt(src, dst, size);
}
