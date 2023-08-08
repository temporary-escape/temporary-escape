#include "controller_poly_shape.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerPolyShape::ControllerPolyShape(entt::registry& reg) : reg{reg} {
}

ControllerPolyShape::~ControllerPolyShape() = default;

void ControllerPolyShape::update(const float delta) {
}

void ControllerPolyShape::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, worldText] : reg.view<ComponentPolyShape>().each()) {
        worldText.recalculate(vulkan);
    }
}
