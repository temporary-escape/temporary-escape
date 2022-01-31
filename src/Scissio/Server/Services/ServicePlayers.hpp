#pragma once

#include "Service.hpp"

namespace Scissio {
class ServicePlayers : public Service {
public:
    explicit ServicePlayers(const Config& config, AssetManager& assetManager, Database& db);
    ~ServicePlayers() override = default;

    std::optional<std::string> secretToId(uint64_t);
    PlayerData login(uint64_t secret, const std::string& name);
    PlayerLocationData findStartingLocation(const PlayerData& player);
    PlayerLocationData getLocation(const std::string& playerId);

    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Scissio
