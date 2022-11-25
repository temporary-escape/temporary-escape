#include "component_script.hpp"

using namespace Engine;

ComponentScript::ComponentScript() = default;

ComponentScript::ComponentScript(Object& object) : Component(object) {
}

ComponentScript::~ComponentScript() = default;
