#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API ControllerCameraOrbital : public Controller, public UserInput {
public:
    explicit ControllerCameraOrbital(Scene& scene, entt::registry& reg);
    ~ControllerCameraOrbital() override;
    NON_COPYABLE(ControllerCameraOrbital);
    NON_MOVEABLE(ControllerCameraOrbital);

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
