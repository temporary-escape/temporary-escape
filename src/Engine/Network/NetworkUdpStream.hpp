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

class ENGINE_API NetworkUdpStream : public NetworkStream {
public:
    explicit NetworkUdpStream(asio::io_service& service, bool isClient);

    bool isEstablished() const {
        return established.load();
    }

    uint64_t getLastPingTime() const {
        return lastPingTime.load();
    }

    uint64_t getSendQueueSize() const {
        return sendQueueSize.load();
    }

    uint64_t getTotalSent() const {
        return totalSent.load();
    }

    uint64_t getTotalReceived() const {
        return totalReceived.load();
    }

protected:
    [[nodiscard]] const std::string& getPublicKey() const {
        return publicKey;
    }

    void forceClosed();
    void sendClosePacket();
    void onReceive(const PacketBytesPtr& packet);
    void onPacketSent(const PacketBytesPtr& packet);

    PacketBytesPtr allocatePacket() override;
    void enqueuePacket(const PacketBytesPtr& packet) override;

    asio::io_service& service;
    asio::io_service::strand strand;
    bool isClient;

private:
    struct SendQueueItem {
        PacketBytesPtr buffer;
        uint32_t time;
    };

    struct ReceiveQueueItem {
        std::array<uint8_t, maxPacketDataSize> buffer{};
        uint32_t length{0};
    };

    using SendQueue = std::array<SendQueueItem, packetQueueSize>;
    using ReceiveQueue = std::array<ReceiveQueueItem, packetQueueSize>;
    using SendQueueList = std::list<SendQueue>;

    void stopAckTimer();
    void stopPingTimer();
    void startAckTimer();
    void startPingTimer();
    void ackReceived(const PacketBytesPtr& packet);
    void sendAck(const PacketBytesPtr& packet);
    void sendPing();
    void sendPong(const PacketBytesPtr& packet);
    void receivePacketReliable(const PacketBytesPtr& packet);
    void receivePacketUnreliable(const PacketBytesPtr& packet);
    void consumePacket(const ReceiveQueueItem& packet);
    void startSendQueue();
    void processQueue();
    void receiveObject(msgpack::object_handle oh);

    virtual void sendPacket(const PacketBytesPtr& packet) = 0;
    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
    virtual std::shared_ptr<NetworkUdpStream> makeShared() = 0;
    virtual void onObjectReceived(msgpack::object_handle oh) = 0;

    ECDH ecdh{};
    std::string publicKey;
    std::vector<uint8_t> sharedSecret;

    std::mutex packetPoolMutex;
    MemoryPool<PacketBytes, 256 * sizeof(PacketBytes)> packetPool{};

    std::atomic<bool> established{false};
    std::atomic<uint64_t> lastPingTime{0};
    std::atomic<uint64_t> sendQueueSize{0};
    std::atomic<uint64_t> totalSent{0};
    std::atomic<uint64_t> totalReceived{0};

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
    asio::steady_timer pingTimer;
    msgpack::unpacker unp;
};
} // namespace Engine
