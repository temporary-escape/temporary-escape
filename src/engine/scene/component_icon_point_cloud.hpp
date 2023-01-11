#pragma once

#include "../assets/image.hpp"
#include "../graphics/shaders/shader_component_point_cloud.hpp"
#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentIconPointCloud : public Component {
public:
    using Point = ShaderComponentPointCloud::Vertex;

    ComponentIconPointCloud() = default;
    virtual ~ComponentIconPointCloud() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentIconPointCloud);

    void add(const Vector3& pos, const Vector2& size, const Color4& color, const ImagePtr& image);

    void recalculate(VulkanRenderer& vulkan);

private:
    std::unordered_map<ImagePtr, std::vector<Point>> imagePoints;
    std::unordered_map<ImagePtr, VulkanBuffer> vbos;
};
} // namespace Engine
