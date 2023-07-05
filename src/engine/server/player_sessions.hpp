#pragma once

#include "../assets/assets_manager.hpp"
#include "messages.hpp"
#include "schemas.hpp"
#include "session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PlayerSessions {
public:
    explicit PlayerSessions(const Config& config, AssetsManager& assetsManager, Database& db);
    virtual ~PlayerSessions() = default;

    // Player data functions
    std::optional<std::string> secretToId(uint64_t);
    PlayerData login(uint64_t secret, const std::string& name);

    // Session functions
    void createSession(const PeerPtr& peer, const std::string& playerId);
    void removeSession(const PeerPtr& peer);
    SessionPtr getSession(const std::string& playerId);
    SessionPtr getSession(const PeerPtr& peer);
    bool isLoggedIn(const std::string& playerId);
    std::vector<SessionPtr> getAllSessions();
    void clear();
    void updateSessionsPing();

private:
    const Config& config;
    AssetsManager& assetsManager;
    Database& db;

    struct {
        std::shared_mutex mutex;
        std::unordered_map<Network::Peer*, SessionPtr> map;
    } sessions;
};
} // namespace Engine
