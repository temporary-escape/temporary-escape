#include "ControllerPolyShape.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerPolyShape::ControllerPolyShape(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerPolyShape::~ControllerPolyShape() = default;

void ControllerPolyShape::update(const float delta) {
}

void ControllerPolyShape::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, polyShape] : reg.view<ComponentPolyShape>().each()) {
        polyShape.recalculate(vulkan);
    }
}
