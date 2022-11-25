#pragma once

#include "world.hpp"

namespace Engine {
class Generator {
public:
    explicit ENGINE_API Generator(const Config& config, TransactionalDatabase& db, World& world);

    void generate(uint64_t seed);
    bool isGenerated();
    void generateGalaxy(uint64_t seed);
    std::vector<RegionData> generateGalaxyRegions(const GalaxyData& galaxy);
    std::vector<SystemData> generateGalaxySystems(const GalaxyData& galaxy, const std::vector<RegionData>& regions);
    void generateSectors(const GalaxyData& galaxy, const std::vector<SystemData>& systems);
    void generateSectors(const GalaxyData& galaxy, const SystemData& system);

private:
    const Config& config;
    TransactionalDatabase& db;
    World& world;
};
} // namespace Engine
