#include "ComponentIcon.hpp"

using namespace Engine;

ComponentIcon::ComponentIcon(entt::registry& reg, entt::entity handle, ImagePtr image) :
    Component{reg, handle}, image{std::move(image)}, color{Colors::primary} {
}

void ComponentIcon::patch(entt::registry& reg, entt::entity handle) {
    reg.patch<ComponentIcon>(handle);
}

void ComponentIcon::setEnvironment(const bool value) {
    if (!value) {
        color = Colors::primary;
    } else {
        color = {0.7f, 0.7f, 0.7f, 0.0f};
    }
    environment = value;
}
