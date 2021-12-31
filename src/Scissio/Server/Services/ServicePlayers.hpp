#pragma once

#include "Service.hpp"

namespace Scissio {
class ServicePlayers : public Service {
public:
    explicit ServicePlayers(const Config& config, AssetManager& assetManager, Database& db);
    ~ServicePlayers() override = default;

    PlayerLocationData getLocation(const std::string& playerId);

    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Scissio
