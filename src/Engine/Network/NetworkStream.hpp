#pragma once

#include "../Crypto/AES.hpp"
#include "../Crypto/HMAC.hpp"
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

static constexpr size_t maxPacketDataSize =
    maxPacketSize - AES::getEncryptSize(0) - sizeof(PacketHeader) - HMAC::resultSize;

class ENGINE_API NetworkStream {
public:
    class Writer;

    using Packer = msgpack::packer<Writer>;

    class ENGINE_API Writer : public Packer {
    public:
        Writer(NetworkStream& stream, PacketType type);

        void write(const char* data, size_t length);
        void flush();

        template <typename T> void start(const uint64_t xid) {
            pack_array(3);
            if constexpr (std::is_same_v<T, std::string>) {
                pack_uint64(0);
            } else {
                pack_uint64(Detail::MessageHelper<T>::hash);
            }
            pack_uint64(xid);
        }

    private:
        NetworkStream& stream;
        PacketType type;
        std::lock_guard<std::mutex> lock;
        std::array<uint8_t, maxPacketDataSize> temp{};
        size_t written{0};
    };

    NetworkStream() = default;
    virtual ~NetworkStream() = default;

    virtual bool isConnected() const = 0;
    virtual const std::string& getAddress() const = 0;
    virtual void close() = 0;

    template <typename T> void send(const T& msg, const uint64_t xid = 1) {
        Writer writer{*this, Detail::MessageHelper<T>::reliable ? PacketType::DataReliable : PacketType::Data};

        writer.start<T>(xid);
        writer.pack(msg);
        writer.flush();
    }

protected:
    virtual PacketBytesPtr allocatePacket() = 0;
    virtual void enqueuePacket(const PacketBytesPtr& packet) = 0;
    void onSharedSecret(const std::vector<uint8_t>& sharedSecret);
    size_t decrypt(const void* src, void* dst, size_t size, bool& verify);

private:
    std::mutex mutex;
    std::unique_ptr<AES> aes;
    std::unique_ptr<HMAC> hmac;
    std::array<uint8_t, HMAC::resultSize> verifyBuffer;
};

template <typename T> inline void BaseRequest2::respond(const T& msg) const {
    peer->send(msg, xid);
}

inline void BaseRequest2::respondError(const std::string& msg) const {
    peer->send(msg, xid);
}
} // namespace Engine
