#pragma once

#include "generator.hpp"

namespace Engine {
class ENGINE_API GeneratorDefault : public Generator {
public:
    using Rng = std::mt19937_64;

    explicit GeneratorDefault(const Config& config, World& world);

    void generate(uint64_t seed) override;
    std::string generateGalaxy(Rng& rng, uint64_t seed);
    void generateGalaxyRegions(const std::string& galaxyId);
    void generateGalaxySystems(const std::string& galaxyId);
    void generateGalaxyFactions(const std::string& galaxyId);
    void generateGalaxySectors(const std::string& galaxyId);

    // bool isGenerated();
    /*void generateGalaxy(uint64_t seed);
    std::vector<RegionData> generateGalaxyRegions(const GalaxyData& galaxy);
    std::vector<SystemData> generateGalaxySystems(const GalaxyData& galaxy, const std::vector<RegionData>& regions);
    void generateSectors(const GalaxyData& galaxy, const std::vector<SystemData>& systems);
    void generateSectors(const GalaxyData& galaxy, const SystemData& system);*/

private:
    void generateSystemDistribution(Rng& rng, const std::string& galaxyId, std::vector<SystemData>& systems,
                                    const std::vector<RegionData>& regions);
    void connectSystems(Rng& rng, std::vector<SystemData>& systems);
    void floodFillRegions(Rng& rng, std::vector<SystemData>& systems, const std::vector<RegionData>& regions);
    void floodFillFactions(Rng& rng, std::vector<SystemData>& systems, const std::vector<FactionData>& factions);
    void populateSystem(Rng& rng, const GalaxyData& galaxy, const SystemData& system);
    void populateSystemPlanets(Rng& rng, const GalaxyData& galaxy, const SystemData& system);

    const Config& config;
    World& world;
};
} // namespace Engine
