#include "controller_text.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerText::ControllerText(entt::registry& reg) : reg{reg} {
}

ControllerText::~ControllerText() = default;

void ControllerText::update(const float delta) {
}

void ControllerText::recalculate(VulkanRenderer& vulkan) {
}
