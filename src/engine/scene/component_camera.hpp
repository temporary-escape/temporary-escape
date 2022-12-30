#pragma once

#include "camera.hpp"
#include "component_user_input.hpp"

namespace Engine {
class ENGINE_API ComponentCamera : public Component, public Camera, public UserInput {
public:
    struct Uniform {
        Matrix4 transformationProjectionMatrix;
        Matrix4 viewProjectionInverseMatrix;
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Vector2i viewport;
        float padding0[2];
        Vector3 eyesPos;
        float padding1[1];
    };

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentCamera() = default;
    explicit ComponentCamera(Object& object) : Component{object}, Camera{object} {
    }
    virtual ~ComponentCamera() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void update(float delta);
    // void render(VulkanDevice& vulkan, const Vector2i& viewport);
    void eventMouseMoved(const Vector2i& pos) override;
    void eventMousePressed(const Vector2i& pos, MouseButton button) override;
    void eventMouseReleased(const Vector2i& pos, MouseButton button) override;
    void eventMouseScroll(int xscroll, int yscroll) override;
    void eventKeyPressed(Key key, Modifiers modifiers) override;
    void eventKeyReleased(Key key, Modifiers modifiers) override;
    void eventCharTyped(uint32_t code) override;

    const VulkanBuffer& getUbo() const {
        return ubo;
    }

    const VulkanBuffer& getUboZeroPos() const {
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

    VulkanBuffer ubo;
    VulkanBuffer uboZeroPos;
    bool move[6]{false};
    float speed{2.0f};
    bool fast{false};
    bool targetAnglesInit{true};
    Vector2 rotationInputValue;
    bool rotationStarted{false};
    bool panFlag{false};
    Vector2i mousePosOld{};
    float zoomMin{0.1f};
    float zoomMax{100.0f};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
