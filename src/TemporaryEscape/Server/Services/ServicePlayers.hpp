#pragma once

#include "Service.hpp"

namespace Engine {
class ENGINE_API ServicePlayers : public Service {
public:
    explicit ServicePlayers(const Config& config, AssetManager& assetManager, Database& db);
    ~ServicePlayers() override = default;

    std::optional<std::string> secretToId(uint64_t);
    PlayerData login(uint64_t secret, const std::string& name);
    PlayerLocationData findStartingLocation(const std::string& playerId);
    PlayerLocationData getLocation(const std::string& playerId);

    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Engine
