#pragma once

#include "../config.hpp"
#include "session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PeerLobby {
public:
    explicit PeerLobby(const Config& config);
    virtual ~PeerLobby() = default;

    void addPeer(const PeerPtr& peer);
    void removePeer(const PeerPtr& peer);
    void disconnectPeer(const PeerPtr& peer);
    std::vector<PeerPtr> getAllPeers();
    void clear();

private:
    const Config& config;

    struct {
        std::shared_mutex mutex;
        std::unordered_set<PeerPtr> map;
    } lobby;
};
} // namespace Engine
