#include "Turret.hpp"
#include "../Server/Lua.hpp"
#include "AssetsManager.hpp"
#include <sol/sol.hpp>

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

void Turret::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<Turret>("Turret");
    cls["name"] = sol::readonly_property(&Turret::getName);
    cls["model"] = sol::readonly_property(&Turret::getModel);
}
