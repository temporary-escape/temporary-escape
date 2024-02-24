#pragma once

#include "../Assets/AssetsManager.hpp"
#include "Messages.hpp"
#include "Schemas.hpp"
#include "Session.hpp"
#include <shared_mutex>

namespace Engine {
class ENGINE_API PlayerSessions {
public:
    explicit PlayerSessions(const Config& config, Database& db);
    virtual ~PlayerSessions() = default;

    // Session functions
    SessionPtr createSession(const NetworkStreamPtr& peer, const std::string& playerId);
    void removeSession(const NetworkStreamPtr& peer);
    SessionPtr getSession(const std::string& playerId);
    SessionPtr getSession(const NetworkStreamPtr& peer);
    bool isLoggedIn(const std::string& playerId);
    std::optional<std::string> getLocation(const SessionPtr& session);
    void setLocation(const SessionPtr& session, const std::optional<std::string>& sectorId);
    std::vector<SessionPtr> getAllSessions();
    void clear();
    // void updateSessionsPing();

private:
    const Config& config;
    Database& db;

    std::shared_mutex mutex;
    std::unordered_map<NetworkStream*, SessionPtr> map;
    std::unordered_map<NetworkStream*, std::string> locations;
};
} // namespace Engine
