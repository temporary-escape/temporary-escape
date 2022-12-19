#pragma once

#include "../assets/texture.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentPointCloud : public Component {
public:
    struct Point {
        Vector3 pos{0.0f};
        Vector2 size{64.0f};
        Color4 color{1.0f};
    };

    static_assert(sizeof(Point) == (3 + 2 + 4) * sizeof(float), "size of Point must be tightly packed");

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentPointCloud() = default;
    explicit ComponentPointCloud(Object& object, TexturePtr texture) : Component{object}, texture{std::move(texture)} {
    }
    virtual ~ComponentPointCloud() = default; // NOLINT(modernize-use-override)

    Delta getDelta() { // NOLINT(readability-convert-member-functions-to-static)
        return {};
    }

    void applyDelta(Delta& delta) { // NOLINT(readability-convert-member-functions-to-static)
        (void)delta;
    }

    void add(const Vector3& pos, const Vector2& size, const Color4& color) {
        setDirty(true);
        points.push_back({pos, size, color});
    }

    void clear() {
        setDirty(true);
        points.clear();
    }

    void recalculate(VulkanDevice& vulkan);
    void render(VulkanDevice& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline);

private:
    TexturePtr texture;
    std::vector<Point> points;
    VulkanBuffer vbo;
    VulkanVertexInputFormat vboFormat;
    size_t count{0};

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
