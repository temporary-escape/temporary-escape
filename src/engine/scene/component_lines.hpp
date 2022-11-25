#pragma once

#include "../library.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentLines : public Component {
public:
    struct Line {
        Vector3 posA;
        Color4 colorA;
        Vector3 posB;
        Color4 colorB;
    };

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentLines() = default;
    explicit ComponentLines(Object& object) : Component(object) {
    }
    explicit ComponentLines(Object& object, std::vector<Line> lines) : Component(object), lines(std::move(lines)) {
        setDirty(true);
    }
    virtual ~ComponentLines() = default;

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    /*const Mesh& getMesh() const {
        return mesh;
    }*/

    void add(const Vector3& from, const Vector3& to, const Color4& color) {
        setDirty(true);
        lines.push_back({from, color, to, color});
    }

    void add(const Vector2& from, const Vector2& to, const Color4& color) {
        setDirty(true);
        lines.push_back({Vector3{from.x, 0.0f, from.y}, color, Vector3{to.x, 0.0f, to.y}, color});
    }

    void recalculate(VulkanDevice& vulkan);
    void render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline);

private:
    std::vector<Line> lines;
    VulkanBuffer vbo;
    VulkanVertexInputFormat vboFormat;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
