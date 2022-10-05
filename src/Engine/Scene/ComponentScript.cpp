#include "ComponentScript.hpp"

using namespace Engine;

ComponentScript::ComponentScript() = default;

ComponentScript::ComponentScript(Object& object) : Component(object) {
}

ComponentScript::~ComponentScript() = default;
