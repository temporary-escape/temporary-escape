
#include "component_2d_selectable.hpp"

using namespace Engine;

Component2DSelectable::Component2DSelectable(entt::registry& reg, entt::entity handle, const Vector2& size) :
    Component{reg, handle}, size{size} {
}
