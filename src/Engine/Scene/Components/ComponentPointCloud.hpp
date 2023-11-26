#pragma once

#include "../../Assets/Texture.hpp"
#include "../../Graphics/Mesh.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentPointCloud : public Component {
public:
    struct Point {
        Vector3 position;
        Vector2 size;
        Vector4 color;
        Vector2 uv;
        Vector2 st;
        Vector2 offset;
        float padding;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Point, position)},
                {1, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, size)},
                {2, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Point, color)},
                {3, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, uv)},
                {4, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, st)},
                {5, VK_FORMAT_R32G32_SFLOAT, offsetof(Point, offset)},
            };
        };
    };

    ComponentPointCloud() = default;
    explicit ComponentPointCloud(entt::registry& reg, entt::entity handle, TexturePtr texture);
    virtual ~ComponentPointCloud() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentPointCloud);

    void reserve(size_t count);
    void add(const Vector3& pos, const Vector2& size, const Color4& color);

    void clear();

    void recalculate(VulkanRenderer& vulkan);

    [[nodiscard]] const TexturePtr& getTexture() const {
        return texture;
    }

    [[nodiscard]] const Mesh& getMesh() const {
        return mesh;
    }

private:
    TexturePtr texture;
    std::vector<Point> points;
    Mesh mesh;
};
} // namespace Engine
