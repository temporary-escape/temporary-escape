#pragma once

#include "../library.hpp"
#include <memory>

namespace Engine {
class ENGINE_API NetworkTcpPeer;

class ENGINE_API NetworkDispatcher {
public:
    virtual ~NetworkDispatcher() = default;

    virtual void onAcceptSuccess(std::shared_ptr<NetworkTcpPeer> peer) = 0;
    virtual void onDisconnect(std::shared_ptr<NetworkTcpPeer> peer) = 0;
};
} // namespace Engine
