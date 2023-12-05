#include "Turret.hpp"
#include "AssetsManager.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Turret::Turret(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void Turret::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)assetsManager;
    (void)audio;

    try {
        Xml::fromFile(this->path, definition);
    } catch (...) {
        EXCEPTION_NESTED("Failed to load turret: '{}'", getName());
    }
}

TurretPtr Turret::from(const std::string& name) {
    return AssetsManager::getInstance().getTurrets().find(name);
}
