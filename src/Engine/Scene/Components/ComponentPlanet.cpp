#include "ComponentPlanet.hpp"

using namespace Engine;

ComponentPlanet::ComponentPlanet(EntityId entity, PlanetTypePtr planetType, const uint64_t seed) :
    Component{entity}, planetType{std::move(planetType)}, seed{seed} {
}
