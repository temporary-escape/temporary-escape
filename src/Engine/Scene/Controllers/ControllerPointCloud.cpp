#include "ControllerPointCloud.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerPointCloud::ControllerPointCloud(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerPointCloud::~ControllerPointCloud() = default;

void ControllerPointCloud::update(const float delta) {
}

void ControllerPointCloud::recalculate(VulkanRenderer& vulkan) {
    for (auto&& [_, pointCloud] : reg.view<ComponentPointCloud>().each()) {
        pointCloud.recalculate(vulkan);
    }
}
