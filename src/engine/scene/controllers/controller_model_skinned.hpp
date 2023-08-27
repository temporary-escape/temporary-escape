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

    [[nodiscard]] const std::vector<Item>& getItems() const {
        return items;
    }

    [[nodiscard]] const VulkanDoubleBuffer& getUbo() const {
        return ubo;
    }

private:
    entt::registry& reg;
    std::vector<Item> items;
    std::vector<ComponentModelSkinned::Armature> armatures;
    VulkanDoubleBuffer ubo;
};
} // namespace Engine
