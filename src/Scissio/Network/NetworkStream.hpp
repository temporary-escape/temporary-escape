#pragma once

#include "Packet.hpp"

namespace Scissio::Network {
class MessageAcceptor;

class SCISSIO_API Stream {
public:
    Stream() = default;
    virtual ~Stream() = default;

    virtual void disconnect() = 0;
    virtual void sendRaw(const Packet& packet) = 0;

    template <typename T> void send(const T& message, const uint64_t sessionId) {
        Packet packet;
        msgpack::pack(packet.data, message);
        packet.id = getMessageId<T>();
        packet.sessionId = sessionId;
        sendRaw(packet);
    }
};

using StreamPtr = std::shared_ptr<Stream>;
} // namespace Scissio::Network
