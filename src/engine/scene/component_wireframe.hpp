#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentWireframe : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentWireframe() = default;
    explicit ComponentWireframe(Object& object);
    explicit ComponentWireframe(Object& object, std::vector<Vector3> vertices, std::vector<uint32_t> indices,
                                const Color4& color);
    virtual ~ComponentWireframe() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }
    void setBox(float width, const Color4& color);
    void render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline);

private:
    std::vector<Vector3> vertices;
    std::vector<uint32_t> indices;
    Color4 color;
    VulkanBuffer vbo;
    VulkanBuffer ibo;
    VulkanVertexInputFormat vboFormat;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
