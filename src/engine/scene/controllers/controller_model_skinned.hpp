#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerModelSkinned : public Controller {
public:
    struct Item {
        entt::entity entity;
        Model* model;
        Matrix4 transform;
        size_t offset;
    };

    explicit ControllerModelSkinned(entt::registry& reg);
    ~ControllerModelSkinned() override;
    NON_COPYABLE(ControllerModelSkinned);
    NON_MOVEABLE(ControllerModelSkinned);

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    [[nodiscard]] const VulkanArrayBuffer& getUbo() const {
        return buffer;
    }

private:
    void addOrUpdate(entt::entity handle, ComponentModelSkinned& component);
    void remove(entt::entity handle);

    void onConstruct(entt::registry& r, entt::entity handle);
    void onUpdate(entt::registry& r, entt::entity handle);
    void onDestroy(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    VulkanArrayBuffer buffer;
};
} // namespace Engine
