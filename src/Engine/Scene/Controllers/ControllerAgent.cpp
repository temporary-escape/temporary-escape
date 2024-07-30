#include "ControllerAgent.hpp"

using namespace Engine;

ControllerAgent::ControllerAgent(Scene& scene, entt::registry& reg) :
    scene{scene}, reg{reg}, rng{std::random_device()()} {
}

ControllerAgent::~ControllerAgent() = default;

void ControllerAgent::update(const float delta) {
    std::vector<AgentObservable> others;
    auto&& everyone = reg.view<ComponentTransform>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, transform] : everyone) {
        if (transform.getParent()) {
            continue;
        }

        auto& other = others.emplace_back();
        other.entity = entity;
        other.position = transform.getPosition();
    }

    AgentWorldState worldState{
        rng,
    };

    auto&& entities = reg.view<ComponentTransform, ComponentAgent>(entt::exclude<TagDisabled>).each();
    for (auto&& [entity, transform, agent] : entities) {
        agent.update(delta, scene, worldState, transform);
    }
}

void ControllerAgent::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}
