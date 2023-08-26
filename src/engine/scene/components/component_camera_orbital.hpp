#pragma once

#include "component_camera.hpp"

namespace Engine {
class ENGINE_API ComponentCameraOrbital : public Component, public UserInput {
public:
    ComponentCameraOrbital() = default;
    explicit ComponentCameraOrbital(entt::registry& reg, entt::entity handle, ComponentCamera& camera);
    virtual ~ComponentCameraOrbital() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentCameraOrbital);

    void update(float delta);
    void setTarget(const Vector3& value);
    void setDistance(float value);
    void setRotation(const Vector2& value);
    void setDistanceRange(float min, float max);

    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    bool isPanning() const {
        return panning;
    }

private:
    ComponentCamera* camera{nullptr};
    bool rotationStarted{false};
    bool panning{false};
    Vector3 target{0.0f, 0.0f, 0.0f};
    float distanceTarget{5.0f};
    float distanceValue{5.0f};
    Vector2 rotation{0.0f, 0.0f};
    Vector2 mousePosOld;
    Vector2 distanceRange{3.0f, 5000.0f};
};
} // namespace Engine
