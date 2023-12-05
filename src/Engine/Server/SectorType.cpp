#include "SectorType.hpp"

using namespace Engine;

bool SectorType::checkConditions(Rng& rng, const GalaxyData& galaxy, const SystemData& system,
                                 const std::vector<PlanetData>& planets) const {
    const auto testWeight = randomReal<float>(rng, 0.0f, 1.0f);
    if (testWeight > weight) {
        return false;
    }

    return true;
}
