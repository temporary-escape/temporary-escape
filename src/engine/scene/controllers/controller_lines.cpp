#include "controller_lines.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerLines::ControllerLines(entt::registry& reg) : reg{reg} {
}

ControllerLines::~ControllerLines() = default;

void ControllerLines::update(const float delta) {
}

void ControllerLines::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, lines] : reg.view<ComponentLines>().each()) {
        lines.recalculate(vulkan);
    }
}
