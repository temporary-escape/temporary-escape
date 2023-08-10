#pragma once

#include "../assets/assets_manager.hpp"
#include "messages.hpp"
#include "schemas.hpp"
#include "session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PlayerSessions {
public:
    explicit PlayerSessions(const Config& config, Database& db);
    virtual ~PlayerSessions() = default;

    // Session functions
    SessionPtr createSession(const NetworkPeerPtr& peer, const std::string& playerId);
    void removeSession(const NetworkPeerPtr& peer);
    SessionPtr getSession(const std::string& playerId);
    SessionPtr getSession(const NetworkPeerPtr& peer);
    bool isLoggedIn(const std::string& playerId);
    std::vector<SessionPtr> getAllSessions();
    void clear();
    // void updateSessionsPing();

private:
    const Config& config;
    Database& db;

    std::shared_mutex mutex;
    std::unordered_map<NetworkPeer*, SessionPtr> map;
};
} // namespace Engine
