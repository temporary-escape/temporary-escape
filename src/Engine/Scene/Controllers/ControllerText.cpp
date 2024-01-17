#include "ControllerText.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerText::ControllerText(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerText::~ControllerText() = default;

void ControllerText::update(const float delta) {
}

void ControllerText::recalculate(VulkanRenderer& vulkan) {
}
