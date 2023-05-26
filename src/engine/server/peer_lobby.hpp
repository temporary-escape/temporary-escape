#pragma once

#include "../config.hpp"
#include "session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PeerLobby {
public:
    explicit PeerLobby(const Config& config);
    virtual ~PeerLobby() = default;

    void addPeerToLobby(const PeerPtr& peer);
    void removePeerFromLobby(const PeerPtr& peer);
    void disconnectPeer(const PeerPtr& peer);
    void clear();

private:
    const Config& config;

    struct {
        std::shared_mutex mutex;
        std::unordered_set<Network::Peer*> map;
    } lobby;
};
} // namespace Engine
