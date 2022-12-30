#pragma once

#include "../assets/image.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentIconPointCloud : public Component {
public:
    struct Point {
        Vector3 pos{0.0f};
        Vector2 size{64.0f};
        Color4 color{1.0f};
        Vector2 uv{0.0f};
        Vector2 st{1.0f};
    };

    static_assert(sizeof(Point) == (3 + 2 + 4 + 4) * sizeof(float), "size of Point must be tightly packed");

    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentIconPointCloud() = default;
    explicit ComponentIconPointCloud(Object& object) : Component{object} {
    }
    virtual ~ComponentIconPointCloud() = default; // NOLINT(modernize-use-override)

    Delta getDelta() { // NOLINT(readability-convert-member-functions-to-static)
        return {};
    }

    void applyDelta(Delta& delta) { // NOLINT(readability-convert-member-functions-to-static)
        (void)delta;
    }

    void add(const Vector3& pos, const Vector2& size, const Color4& color, const ImagePtr& image) {
        setDirty(true);
        imagePoints[image].push_back({pos, size, color, image->getAllocation().uv, image->getAllocation().st});
    }

    void recalculate(VulkanRenderer& vulkan);
    void render(VulkanRenderer& vulkan, const Vector2i& viewport, VulkanPipeline& pipeline);

private:
    std::unordered_map<ImagePtr, std::vector<Point>> imagePoints;
    std::unordered_map<ImagePtr, VulkanBuffer> vbos;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
