#pragma once

#include "Component.hpp"

namespace Engine {
class ENGINE_API ComponentDebug : public Component {
public:
    struct Vertex {
        Vector3 position;
        Color4 color;
    };

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentDebug() = default;
    explicit ComponentDebug(Object& object) : Component(object) {
    }

    virtual ~ComponentDebug() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    void render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline);

    void clear();
    void addBox(const Matrix4& transform, float width, const Color4& color);

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    VulkanVertexInputFormat vboFormat;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
