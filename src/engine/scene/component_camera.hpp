#pragma once

#include "../graphics/shader.hpp"
#include "camera.hpp"
#include "component_transform.hpp"
#include "component_user_input.hpp"

namespace Engine {
class ENGINE_API ComponentCamera : public Component, public Camera, public UserInput {
public:
    using Uniform = Shader::CameraUniform;

    ComponentCamera() = default;
    explicit ComponentCamera(ComponentTransform& transform) : Camera{transform.getTransform()} {
    }
    virtual ~ComponentCamera() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentCamera);

    void update(float delta);
    void recalculate(VulkanRenderer& vulkan, const Vector2i& viewport);
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    [[nodiscard]] const VulkanDoubleBuffer& getUbo() const {
        return ubo;
    }

    [[nodiscard]] const VulkanDoubleBuffer& getUboZeroPos() const {
        return uboZeroPos;
    }

    void setSpeed(float value) {
        speed = value;
    }

    void setZoomRange(const float min, const float max) {
        zoomMin = min;
        zoomMax = max;
    }

private:
    void updateRotationFreeLook(const Vector2& diff);
    void moveToOrtographic(const Vector3& position);

    VulkanDoubleBuffer ubo;
    VulkanDoubleBuffer uboZeroPos;
    bool move[6]{false};
    float speed{2.0f};
    bool fast{false};
    bool targetAnglesInit{true};
    Vector2 rotationInputValue{};
    bool rotationStarted{false};
    bool panFlag{false};
    Vector2i mousePosOld{};
    float zoomMin{0.1f};
    float zoomMax{100.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
