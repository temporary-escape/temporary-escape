#include "ControllerCamera.hpp"

using namespace Engine;

ControllerCamera::ControllerCamera(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
}

ControllerCamera::~ControllerCamera() = default;

void ControllerCamera::update(const float delta) {
    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.update(delta);
    }
}

void ControllerCamera::recalculate(VulkanRenderer& vulkan) {
    if (!descriptorPool) {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT |
                             VK_SHADER_STAGE_COMPUTE_BIT;

        descriptorPool = VulkanDescriptorSetPool{vulkan, {&binding, 1}, 16};
    }

    auto entities = reg.view<ComponentCamera>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, camera] : entities) {
        camera.recalculate(vulkan, descriptorPool);
    }
}
