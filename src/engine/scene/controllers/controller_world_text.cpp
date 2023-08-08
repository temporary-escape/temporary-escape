#include "controller_world_text.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerWorldText::ControllerWorldText(entt::registry& reg) : reg{reg} {
}

ControllerWorldText::~ControllerWorldText() = default;

void ControllerWorldText::update(const float delta) {
}

void ControllerWorldText::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, worldText] : reg.view<ComponentWorldText>().each()) {
        worldText.recalculate(vulkan);
    }
}
