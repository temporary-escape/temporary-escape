#pragma once

#include "Service.hpp"

namespace Scissio {
class ServiceRegions : public Service {
public:
    explicit ServiceRegions(const Config& config, AssetManager& assetManager, Database& db);
    ~ServiceRegions() override = default;

    void generate();
    void generate(const std::string& galaxyId);
    void tick() override;

private:
    const Config& config;
    AssetManager& assetManager;
    Database& db;
};
} // namespace Scissio
