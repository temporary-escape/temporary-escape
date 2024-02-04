#pragma once

#include "../Crypto/AES.hpp"
#include "../Crypto/ECDH.hpp"
#include "../Utils/MemoryPool.hpp"
#include "NetworkMessage.hpp"
#include "NetworkPacket.hpp"
#include "NetworkUtils.hpp"
#include <list>
#include <mutex>

namespace Engine {
// using PacketBuffer = std::array<uint8_t, maxPacketSize>;
static constexpr size_t maxPacketDataSize = maxPacketSize - AES::getEncryptSize(0) - sizeof(PacketHeader);
static constexpr size_t packetQueueSize = 32;
static constexpr size_t packetWindowSize = 8;
/*struct PacketData {
    PacketBuffer buffer;
    size_t length;
};*/

// using PacketDataPtr = std::shared_ptr<PacketData>;

// using PacketBuffer = std::vector<uint8_t>;
// using PacketBufferPtr = std::shared_ptr<std::vector<uint8_t>>;

struct PacketBytes {
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

class NetworkUdpConnection {
public:
    class Writer;

    using Packer = msgpack::packer<Writer>;

    class Writer : public Packer {
    public:
        Writer(NetworkUdpConnection& conn, PacketType type);

        void write(const char* data, size_t length);
        void flush();

    private:
        NetworkUdpConnection& conn;
        PacketType type;
        std::lock_guard<std::mutex> lock;
        std::array<uint8_t, maxPacketDataSize> temp{};
        size_t written{0};
    };

    NetworkUdpConnection(asio::io_service& service);
    virtual ~NetworkUdpConnection() = default;

    template <typename T> void send(const T& msg, const uint64_t xid) {
        Writer writer{*this, PacketType::DataReliable};

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

    [[nodiscard]] const std::string& getPublicKey() const {
        return publicKey;
    }

    [[nodiscard]] size_t getSequenceNum() const {
        return sequenceNum;
    }

    [[nodiscard]] size_t getSendNum() const {
        return sendNum;
    }

    [[nodiscard]] size_t getAckNum() const {
        return ackNum;
    }

protected:
    // void onSentPacket(PacketData* packet, bool error);
    void onReceive(const PacketBytesPtr& packet);
    void onPacketSent(const PacketBytesPtr& packet);
    PacketBytesPtr allocatePacket();

    asio::io_service& service;
    asio::io_service::strand strand;

private:
    using SendQueue = std::array<PacketBytesPtr, packetQueueSize>;
    using SendQueueList = std::list<SendQueue>;

    void enqueuePacket(const PacketBytesPtr& packet);
    void onAckReceived(const PacketBytesPtr& packet);
    void sendAck(const PacketBytesPtr& packet);
    void startSendQueue();

    virtual void sendPacket(const PacketBytesPtr& packet) = 0;
    virtual void onConnected() = 0;
    virtual std::shared_ptr<NetworkUdpConnection> makeShared() = 0;

    ECDH ecdh{};
    std::string publicKey;
    std::vector<uint8_t> sharedSecret;
    std::unique_ptr<AES> aes;

    std::mutex packetPoolMutex;
    MemoryPool<PacketBytes, 64 * 1024> packetPool{};

    std::mutex writeMutex;

    // MemoryPool<PacketData, sizeof(PacketData) * 64> memoryPool;
    uint32_t sequenceNum{0};
    uint32_t ackNum{0};
    uint32_t sendNum{0};
    std::array<uint8_t, maxPacketDataSize> plaintext{};
    SendQueueList sendQueueList;
    SendQueueList::iterator windowPos;
    SendQueueList::iterator enqueuePos;
    bool sending{false};
    // std::array<PacketBufferPtr, packetQueueSize> sendQueue{};
    // std::array<PacketBufferPtr, packetQueueSize> receiveQueue{};

    uint32_t receiveNum{0};
};
} // namespace Engine
