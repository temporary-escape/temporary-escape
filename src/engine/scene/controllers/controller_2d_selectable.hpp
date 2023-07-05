#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API Controller2DSelectable : public Controller, public UserInput {
public:
    using Point = Vector4;
    using Output = Vector2;

    explicit Controller2DSelectable(entt::registry& reg);
    ~Controller2DSelectable() override;

    void update(float delta) override;

    void recalculate(VulkanRenderer& vulkan) override;

    const VulkanBuffer& getBufferInput() const {
        return sboInput;
    }

    const VulkanBuffer& getBufferOutput() const {
        return sboOutput.getCurrentBuffer();
    }

    size_t getCount() const {
        return components.size();
    }

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

private:
    void onConstruct(entt::registry& r, entt::entity handle);
    std::optional<size_t> findNearestPoint(const Vector2i& pos);

    entt::registry& reg;

    VulkanBuffer sboInput;
    VulkanDoubleBuffer sboOutput;
    std::vector<const Component2DSelectable*> components;
    bool recreate{false};
    std::optional<MouseButton> mousePressed;
    Vector2i mousePressedPos;
    std::optional<size_t> hovered;
};
} // namespace Engine
