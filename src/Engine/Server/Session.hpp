#pragma once

#include "../Network/NetworkTcpPeer.hpp"
#include "Messages.hpp"

namespace Engine {
class ENGINE_API Server;

class ENGINE_API Session {
public:
    explicit Session(std::string playerId, const std::shared_ptr<NetworkTcpPeer<Server, ServerSink>>& stream)
        : playerId(std::move(playerId)), stream(stream) {
    }

    const std::string& getPlayerId() const {
        return playerId;
    }

    template <typename T> void send(T& msg) {
        // stream->send(msg);
        if (auto ptr = stream.lock(); ptr != nullptr) {
            ptr->send(msg);
        }
    }

private:
    std::string playerId;
    std::weak_ptr<NetworkTcpPeer<Server, ServerSink>> stream;
};

using SessionPtr = std::shared_ptr<Session>;
} // namespace Engine
