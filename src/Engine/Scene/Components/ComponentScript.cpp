#include "ComponentScript.hpp"
#include "../../Server/Lua.hpp"
#include "../Entity.hpp"
#include <sol/sol.hpp>

using namespace Engine;

struct ComponentScript::Data {
    sol::table instance;
};

ComponentScript::ComponentScript() = default;

ComponentScript::ComponentScript(entt::registry& reg, entt::entity handle, const sol::table& instance) :
    Component{reg, handle}, data{std::make_unique<Data>()} {

    data->instance = instance;
}

ComponentScript::~ComponentScript() noexcept = default;

Engine::ComponentScript::ComponentScript(ComponentScript&& other) noexcept = default;

ComponentScript& Engine::ComponentScript::operator=(ComponentScript&& other) noexcept = default;

sol::table& Engine::ComponentScript::getInstance() const {
    return data->instance;
}

void ComponentScript::bind(Lua& lua) {
    auto& m = lua.root();

    auto cls = m.new_usertype<ComponentScript>("ComponentScript");
    cls["instance"] = sol::readonly_property(&ComponentScript::getInstance);
}
