#include "ComponentPlanet.hpp"

using namespace Engine;

ComponentPlanet::ComponentPlanet(entt::registry& reg, entt::entity handle, PlanetTypePtr planetType,
                                 const uint64_t seed) :
    Component{reg, handle}, planetType{std::move(planetType)}, seed{seed} {
}
