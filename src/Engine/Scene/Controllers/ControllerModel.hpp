#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerModel : public Controller {
public:
    explicit ControllerModel(entt::registry& reg);
    ~ControllerModel() override;
    NON_COPYABLE(ControllerModel);
    NON_MOVEABLE(ControllerModel);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

private:
    void addOrUpdate(entt::entity handle, ComponentModel& component);

    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    entt::registry& reg;
};
} // namespace Engine
