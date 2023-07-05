#include "component_icon.hpp"

using namespace Engine;

ComponentIcon::ComponentIcon(entt::registry& reg, entt::entity handle, ImagePtr image, const Vector2& size,
                             const Color4& color) :
    Component{reg, handle}, image{std::move(image)}, size{size}, color{color} {
}
