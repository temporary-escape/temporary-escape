#include "ControllerWorldText.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerWorldText::ControllerWorldText(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerWorldText::~ControllerWorldText() = default;

void ControllerWorldText::update(const float delta) {
}

void ControllerWorldText::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, worldText] : reg.view<ComponentWorldText>().each()) {
        worldText.recalculate(vulkan);
    }
}
