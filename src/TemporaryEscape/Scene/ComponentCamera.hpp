#pragma once

#include "Camera.hpp"
#include "Component.hpp"
#include "ComponentUserInput.hpp"

namespace Engine {
class ENGINE_API ComponentCamera : public Component, public Camera, public UserInput::Handler {
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
    void render(VulkanDevice& vulkan, const Vector2i& viewport);
    void eventUserInput(const UserInput::Event& event) override;

    const VulkanBuffer& getUbo() const {
        return ubo;
    }

    void setSpeed(float value) {
        speed = value;
    }

private:
    void updateRotationFreeLook(const Vector2& diff);

    VulkanBuffer ubo;
    bool move[6]{false};
    float speed{2.0f};
    bool fast{false};
    bool targetAnglesInit{true};
    Vector2 rotationInputValue;
    bool rotationStarted{false};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
