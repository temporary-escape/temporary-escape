#pragma once

#include "NetworkStream.hpp"

namespace Scissio::Network {
class EventListener {
public:
    virtual void eventPacket(const StreamPtr& stream, Packet packet) = 0;
    virtual void eventConnect(const StreamPtr& stream) = 0;
    virtual void eventDisconnect(const StreamPtr& stream) = 0;
};
} // namespace Scissio::Network
