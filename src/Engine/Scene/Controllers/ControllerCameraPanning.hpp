#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerCameraPanning : public Controller, public UserInput {
public:
    explicit ControllerCameraPanning(Scene& scene, entt::registry& reg);
    ~ControllerCameraPanning() override;
    NON_COPYABLE(ControllerCameraPanning);
    NON_MOVEABLE(ControllerCameraPanning);

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
    Scene& scene;
    entt::registry& reg;
};
} // namespace Engine
