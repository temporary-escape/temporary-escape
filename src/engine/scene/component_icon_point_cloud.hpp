#pragma once

#include "../assets/image.hpp"
#include "../graphics/mesh.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentIconPointCloud : public Component {
public:
    struct Point {
        Vector3 position;
        Vector2 size;
        Vector4 color;
        Vector2 uv;
        Vector2 st;
    };

    ComponentIconPointCloud() = default;
    virtual ~ComponentIconPointCloud() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentIconPointCloud);

    void add(const Vector3& pos, const Vector2& size, const Color4& color, const ImagePtr& image);

    void recalculate(VulkanRenderer& vulkan);

    const std::unordered_map<ImagePtr, Mesh>& getMesges() const {
        return meshes;
    }

private:
    std::unordered_map<ImagePtr, std::vector<Point>> imagePoints;
    std::unordered_map<ImagePtr, Mesh> meshes;
};
} // namespace Engine
