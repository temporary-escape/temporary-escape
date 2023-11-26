#pragma once

#include "../Config.hpp"
#include "Session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PeerLobby {
public:
    explicit PeerLobby(const Config& config);
    virtual ~PeerLobby() = default;

    void addPeer(const NetworkPeerPtr& peer);
    void removePeer(const NetworkPeerPtr& peer);
    void disconnectPeer(const NetworkPeerPtr& peer);
    std::vector<NetworkPeerPtr> getAllPeers();
    void clear();

private:
    const Config& config;

    std::shared_mutex mutex;
    std::vector<NetworkPeerPtr> peers;
};
} // namespace Engine
