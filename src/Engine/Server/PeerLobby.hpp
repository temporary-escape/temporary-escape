#pragma once

#include "../Config.hpp"
#include "Session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PeerLobby {
public:
    explicit PeerLobby(const Config& config);
    virtual ~PeerLobby() = default;

    void addPeer(const NetworkStreamPtr& peer);
    void removePeer(const NetworkStreamPtr& peer);
    void disconnectPeer(const NetworkStreamPtr& peer);
    std::vector<NetworkStreamPtr> getAllPeers();
    void clear();

private:
    const Config& config;

    std::shared_mutex mutex;
    std::vector<NetworkStreamPtr> peers;
};
} // namespace Engine
