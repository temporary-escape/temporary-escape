#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API ControllerCamera : public Controller, public UserInput {
public:
    explicit ControllerCamera(entt::registry& reg);
    ~ControllerCamera() override;
    NON_COPYABLE(ControllerCamera);
    NON_MOVEABLE(ControllerCamera);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

private:
    entt::registry& reg;
};
} // namespace Engine
