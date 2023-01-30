#include "planet_type.hpp"
#include "registry.hpp"

#define CMP "Block"

using namespace Engine;

PlanetType::PlanetType(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
    try {
        definition.fromYaml(this->path);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load planet type: '{}'", getName());
    }
}

void PlanetType::load(Registry& registry, VulkanRenderer& vulkan) {
}

PlanetTypePtr PlanetType::from(const std::string& name) {
    return Registry::getInstance().getPlanetTypes().find(name);
}
