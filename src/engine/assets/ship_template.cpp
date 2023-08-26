#include "ship_template.hpp"
#include "../file/msgpack_file_reader.hpp"
#include "../file/teb_file_header.hpp"
#include "../server/lua.hpp"
#include "assets_manager.hpp"
#include <sol/sol.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ShipTemplate::ShipTemplate(std::string name, Path path) : Asset{std::move(name)}, path{std::move(path)} {
}

void ShipTemplate::load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) {
    (void)assetsManager;
    (void)vulkan;
    (void)audio;

    try {
        MsgpackFileReader file{path};
        TebFileHeader header{};
        file.unpack(header);
        if (header.type != TebFileType::Ship) {
            EXCEPTION("File is not a ship template");
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to load ship template: '{}'", getName());
    }
}

ShipTemplatePtr ShipTemplate::from(const std::string& name) {
    return AssetsManager::getInstance().getShipTemplates().find(name);
}

void ShipTemplate::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ShipTemplate>("ShipTemplate");
    cls["name"] = sol::property(&ShipTemplate::getName);
}
