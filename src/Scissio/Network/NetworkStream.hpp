#pragma once

#include "Packet.hpp"

namespace Scissio::Network {
class SCISSIO_API Stream {
public:
    Stream() = default;
    virtual ~Stream() = default;

    virtual void disconnect() = 0;
    virtual void sendRaw(const Packet& packet) = 0;

    template <typename T> void send(const T& message) {
        Packet packet;
        msgpack::pack(packet.data, message);
        packet.id = getMessageId<T>();
        sendRaw(packet);
    }
};

using StreamPtr = std::shared_ptr<Stream>;
} // namespace Scissio::Network
