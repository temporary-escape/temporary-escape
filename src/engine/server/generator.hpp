#pragma once

#include <utility>

#include "../scene/scene.hpp"
#include "world.hpp"

namespace Engine {
class ENGINE_API Generator {
public:
    using Rng = std::mt19937_64;

    explicit Generator(const Config& config, World& world);

    void generate(uint64_t seed);

    // void generate(uint64_t seed);
    // std::string generateGalaxy(Rng& rng, uint64_t seed);
    // void generateGalaxyRegions(const std::string& galaxyId);
    // void generateGalaxySystems(const std::string& galaxyId);
    // void generateGalaxyFactions(const std::string& galaxyId);
    // void generateGalaxySectors(const std::string& galaxyId);

private:
    // void generateSystemDistribution(Rng& rng, const std::string& galaxyId, std::vector<SystemData>& systems,
    //                                 const std::vector<RegionData>& regions);
    // void connectSystems(Rng& rng, std::vector<SystemData>& systems);
    // void floodFillRegions(Rng& rng, std::vector<SystemData>& systems, const std::vector<RegionData>& regions);
    // void floodFillFactions(Rng& rng, std::vector<SystemData>& systems, const std::vector<FactionData>& factions);
    // void populateSystem(Rng& rng, const GalaxyData& galaxy, const SystemData& system);
    // void populateSystemPlanets(Rng& rng, const GalaxyData& galaxy, const SystemData& system);

    const Config& config;
    World& world;
};
} // namespace Engine
