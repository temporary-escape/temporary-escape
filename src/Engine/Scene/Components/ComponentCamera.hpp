#pragma once

#include "../Camera.hpp"
#include "ComponentTransform.hpp"

namespace Engine {
class ENGINE_API ComponentCamera : public Component, public Camera {
public:
    ComponentCamera() = default;
    explicit ComponentCamera(entt::registry& reg, entt::entity handle, ComponentTransform& transform);
    virtual ~ComponentCamera() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentCamera);

    void update(float delta);
    void recalculate(VulkanRenderer& vulkan, const Vector2i& viewport);

    [[nodiscard]] const VulkanDoubleBuffer& getUbo() const {
        return ubo;
    }

    [[nodiscard]] const VulkanDoubleBuffer& getUboZeroPos() const {
        return uboZeroPos;
    }

    /*void setSpeed(float value) {
        speed = value;
    }*/

    /*void setOrthographic(const float zoom) override {
        Camera::setOrthographic(zoom);
        zoomValue = zoom;
        zoomTarget = zoom;
    }*/

    /*void setZoomRange(const float min, const float max) {
        zoomMin = min;
        zoomMax = max;
    }*/

    bool isPanning() const {
        return panning;
    }

    void setPanning(bool value) {
        panning = value;
    }

private:
    // void updateRotationFreeLook(const Vector2& diff);
    // void moveToOrtographic(const Vector3& position);

    VulkanDoubleBuffer ubo;
    VulkanDoubleBuffer uboZeroPos;
    bool panning{false};
    /*bool move[6]{false};
    float speed{2.0f};
    bool fast{false};
    Vector2 rotationInputValue{};
    bool rotationStarted{false};
    bool panFlag{false};
    bool panning{false};
    Vector2i mousePosOld{};
    float zoomMin{0.1f};
    float zoomMax{100.0f};
    float zoomTarget{0.0f};
    float zoomValue{0.0f};*/

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
