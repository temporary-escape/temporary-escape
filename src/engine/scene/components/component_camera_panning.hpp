#pragma once

#include "component_camera.hpp"

namespace Engine {
class ENGINE_API ComponentCameraPanning : public Component, public UserInput {
public:
    ComponentCameraPanning() = default;
    explicit ComponentCameraPanning(entt::registry& reg, entt::entity handle, ComponentCamera& camera);
    virtual ~ComponentCameraPanning() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentCameraPanning);

    void update(float delta);
    void setTarget(const Vector3& value);
    void setDistance(float value);
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
    bool panFlag{false};
    bool panning{false};
    float zoomTarget{0.0f};
    float zoomValue{0.0f};
    Vector2 zoomRange{0.1f, 200.0f};
    Vector2 mousePosOld;
};
} // namespace Engine
