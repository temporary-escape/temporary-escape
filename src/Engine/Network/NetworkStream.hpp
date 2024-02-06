#pragma once

#include "../Crypto/AES.hpp"
#include "NetworkMessage.hpp"
#include "NetworkPacket.hpp"
#include <mutex>

namespace Engine {
struct ENGINE_API PacketBytes {
    std::array<uint8_t, maxPacketSize> buffer;
    size_t length;

    [[nodiscard]] uint8_t* data() {
        return buffer.data();
    }

    [[nodiscard]] const uint8_t* data() const {
        return buffer.data();
    }

    [[nodiscard]] size_t size() const {
        return length;
    }
};

using PacketBytesPtr = std::shared_ptr<PacketBytes>;

static constexpr size_t maxPacketDataSize = maxPacketSize - AES::getEncryptSize(0) - sizeof(PacketHeader);

class ENGINE_API NetworkStream {
public:
    class Writer;

    using Packer = msgpack::packer<Writer>;

    class ENGINE_API Writer : public Packer {
    public:
        Writer(NetworkStream& stream, PacketType type);

        void write(const char* data, size_t length);
        void flush();

    private:
        NetworkStream& stream;
        PacketType type;
        std::lock_guard<std::mutex> lock;
        std::array<uint8_t, maxPacketDataSize> temp{};
        size_t written{0};
    };

    NetworkStream() = default;
    virtual ~NetworkStream() = default;

    template <typename T> void send(const T& msg, const uint64_t xid) {
        Writer writer{*this, Detail::MessageHelper<T>::reliable ? PacketType::DataReliable : PacketType::Data};

        writer.pack_array(3);
        if constexpr (std::is_same_v<T, std::string>) {
            writer.pack_uint64(0);
        } else {
            writer.pack_uint64(Detail::MessageHelper<T>::hash);
        }
        writer.pack_uint64(xid);
        writer.pack(msg);
        writer.flush();
    }

protected:
    virtual PacketBytesPtr allocatePacket() = 0;
    virtual void enqueuePacket(const PacketBytesPtr& packet) = 0;
    void onSharedSecret(const std::vector<uint8_t>& sharedSecret);
    size_t decrypt(const void* src, void* dst, size_t size);

private:
    std::mutex mutex;
    std::unique_ptr<AES> aes;
};
} // namespace Engine
