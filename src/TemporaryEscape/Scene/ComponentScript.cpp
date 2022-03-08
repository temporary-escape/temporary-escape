#include "ComponentScript.hpp"
#include <wrenbind17/wrenbind17.hpp>

using namespace Engine;

ComponentScript::ComponentScript() = default;

ComponentScript::ComponentScript(Object& object) : Component(object) {
}

ComponentScript::~ComponentScript() = default;
