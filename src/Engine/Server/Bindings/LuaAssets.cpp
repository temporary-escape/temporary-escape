#include "../../Assets/AssetsManager.hpp"
#include "Bindings.hpp"

using namespace Engine;

static void bindAssetsManager(sol::table& m) {
    auto cls = m.new_usertype<AssetsManager>("AssetsManager");
    cls["find_planet_type"] = [](AssetsManager& self, const std::string& name) {
        return self.getPlanetTypes().find(name);
    };
    cls["find_image"] = [](AssetsManager& self, const std::string& name) { return self.getImages().find(name); };
    cls["find_texture"] = [](AssetsManager& self, const std::string& name) { return self.getTextures().find(name); };
    cls["find_block"] = [](AssetsManager& self, const std::string& name) { return self.getBlocks().find(name); };
    cls["find_model"] = [](AssetsManager& self, const std::string& name) { return self.getModels().find(name); };
    cls["find_sound"] = [](AssetsManager& self, const std::string& name) { return self.getSounds().find(name); };
    cls["find_particles_type"] = [](AssetsManager& self, const std::string& name) {
        return self.getParticlesTypes().find(name);
    };
    cls["find_ship_template"] = [](AssetsManager& self, const std::string& name) {
        return self.getShipTemplates().find(name);
    };
    cls["find_turret"] = [](AssetsManager& self, const std::string& name) { return self.getTurrets().find(name); };

    cls["find_all_planet_types"] = [](AssetsManager& self) { return sol::as_table(self.getPlanetTypes().findAll()); };
    cls["find_all_images"] = [](AssetsManager& self) { return sol::as_table(self.getImages().findAll()); };
    cls["find_all_textures"] = [](AssetsManager& self) { return sol::as_table(self.getTextures().findAll()); };
    cls["find_all_blocks"] = [](AssetsManager& self) { return sol::as_table(self.getBlocks().findAll()); };
    cls["find_all_models"] = [](AssetsManager& self) { return sol::as_table(self.getModels().findAll()); };
    cls["find_all_sounds"] = [](AssetsManager& self) { return sol::as_table(self.getSounds().findAll()); };
    cls["find_all_particles_type"] = [](AssetsManager& self) {
        return sol::as_table(self.getParticlesTypes().findAll());
    };
    cls["find_all_ship_templates"] = [](AssetsManager& self) {
        return sol::as_table(self.getShipTemplates().findAll());
    };
    cls["find_all_turrets"] = [](AssetsManager& self) { return sol::as_table(self.getTurrets().findAll()); };
}

LUA_BINDINGS(bindAssetsManager);

static void bindBlock(sol::table& m) {
    auto cls = m.new_usertype<Block>("Block");
    cls["name"] = sol::property(&Block::getName);
}

LUA_BINDINGS(bindBlock);

static void bindImage(sol::table& m) {
    auto cls = m.new_usertype<Image>("Image");
    cls["name"] = sol::property(&Image::getName);
}

LUA_BINDINGS(bindImage);

static void bindModel(sol::table& m) {
    auto cls = m.new_usertype<Model>("Model");
    cls["name"] = sol::readonly_property(&Model::getName);
    cls["radius"] = sol::readonly_property(&Model::getRadius);
}

LUA_BINDINGS(bindModel);

static void bindParticlesType(sol::table& m) {
    auto cls = m.new_usertype<ParticlesType>("ParticlesType");
    cls["name"] = sol::property(&ParticlesType::getName);
}

LUA_BINDINGS(bindParticlesType);

static void bindPlanetType(sol::table& m) {
    auto cls = m.new_usertype<PlanetType>("PlanetType");
    cls["name"] = sol::property(&PlanetType::getName);
}

LUA_BINDINGS(bindPlanetType);

static void bindShipTemplate(sol::table& m) {
    auto cls = m.new_usertype<ShipTemplate>("ShipTemplate");
    cls["name"] = sol::property(&ShipTemplate::getName);
}

LUA_BINDINGS(bindShipTemplate);

static void bindSound(sol::table& m) {
    auto cls = m.new_usertype<Sound>("Sound");
    cls["name"] = sol::property(&Sound::getName);
}

LUA_BINDINGS(bindSound);

static void bindTexture(sol::table& m) {
    auto cls = m.new_usertype<Texture>("Texture");
    cls["name"] = sol::property(&Texture::getName);
}

LUA_BINDINGS(bindTexture);

static void bindTurret(sol::table& m) {
    auto cls = m.new_usertype<Turret>("Turret");
    cls["name"] = sol::readonly_property(&Turret::getName);
    cls["model"] = sol::readonly_property(&Turret::getModel);
}

LUA_BINDINGS(bindTurret);
