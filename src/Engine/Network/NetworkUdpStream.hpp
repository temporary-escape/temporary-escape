#pragma once

#include "../Crypto/ECDH.hpp"
#include "../Utils/MemoryPool.hpp"
#include "NetworkMessage.hpp"
#include "NetworkPacket.hpp"
#include "NetworkStream.hpp"
#include "NetworkUtils.hpp"
#include <list>

namespace Engine {
static constexpr size_t packetQueueSize = 256;
static constexpr size_t packetWindowSize = 64;

using PacketBytesPtr = std::shared_ptr<PacketBytes>;

class NetworkUdpStream : public NetworkStream {
public:
    NetworkUdpStream(asio::io_service& service);
    virtual ~NetworkUdpStream() = default;

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
    void stopAckTimer();
    void startAckTimer();
    void onReceive(const PacketBytesPtr& packet);
    void onPacketSent(const PacketBytesPtr& packet);

    PacketBytesPtr allocatePacket() override;
    void enqueuePacket(const PacketBytesPtr& packet) override;

    asio::io_service& service;
    asio::io_service::strand strand;

private:
    struct SendQueueItem {
        PacketBytesPtr buffer;
        uint32_t time;
    };

    struct ReceiveQueueItem {
        std::array<uint8_t, maxPacketDataSize> buffer;
        uint32_t length{0};
    };

    using SendQueue = std::array<SendQueueItem, packetQueueSize>;
    using ReceiveQueue = std::array<ReceiveQueueItem, packetQueueSize>;
    using SendQueueList = std::list<SendQueue>;

    void onAckReceived(const PacketBytesPtr& packet);
    void sendAck(const PacketBytesPtr& packet);
    void receivePacket(const PacketBytesPtr& packet);
    void consumePacket(const ReceiveQueueItem& packet);
    void startSendQueue();
    void processQueue();
    void receiveObject(msgpack::object_handle oh);

    virtual void sendPacket(const PacketBytesPtr& packet) = 0;
    virtual void onConnected() = 0;
    virtual std::shared_ptr<NetworkUdpStream> makeShared() = 0;
    virtual void onObjectReceived(msgpack::object_handle oh) = 0;

    ECDH ecdh{};
    std::string publicKey;
    std::vector<uint8_t> sharedSecret;

    std::mutex packetPoolMutex;
    MemoryPool<PacketBytes, 256 * sizeof(PacketBytes)> packetPool{};

    uint32_t sequenceNum{0};
    uint32_t ackNum{0};
    uint32_t sendNum{0};
    std::array<uint8_t, maxPacketDataSize> plaintext{};
    SendQueueList sendQueueList;
    SendQueueList::iterator windowPos;
    SendQueueList::iterator enqueuePos;
    ReceiveQueue receiveQueue{};
    bool sending{false};

    uint32_t receiveNum{0};
    asio::steady_timer ackTimer;
    msgpack::unpacker unp;
};
} // namespace Engine
