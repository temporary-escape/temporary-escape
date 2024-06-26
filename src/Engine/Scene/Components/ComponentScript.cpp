#include "ComponentScript.hpp"
#include "../Entity.hpp"
#include <sol/sol.hpp>

using namespace Engine;

struct ComponentScript::Data {
    sol::table instance;
};

ComponentScript::ComponentScript() = default;

ComponentScript::ComponentScript(EntityId entity, const sol::table& instance) :
    Component{entity}, data{std::make_unique<Data>()} {

    data->instance = instance;
}

ComponentScript::~ComponentScript() noexcept = default;

Engine::ComponentScript::ComponentScript(ComponentScript&& other) noexcept = default;

ComponentScript& Engine::ComponentScript::operator=(ComponentScript&& other) noexcept = default;

sol::table& Engine::ComponentScript::getInstance() const {
    return data->instance;
}
