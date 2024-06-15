#include "ComponentIcon.hpp"

using namespace Engine;

ComponentIcon::ComponentIcon(EntityId entity, ImagePtr image) :
    Component{entity}, image{std::move(image)}, color{Colors::yellow} {
}

void ComponentIcon::setEnvironment(const bool value) {
    if (!value) {
        color = Colors::yellow;
    } else {
        color = {0.7f, 0.7f, 0.7f, 0.0f};
    }
    environment = value;
}
