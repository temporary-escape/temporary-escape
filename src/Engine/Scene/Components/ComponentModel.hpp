#pragma once

#include "../../Assets/Model.hpp"
#include "../Component.hpp"

namespace Engine {
class ENGINE_API ComponentModel : public Component {
public:
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector4 tangent;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
                {2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords)},
                {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)},
            };
        };
    };

    struct InstancedVertex {
        Vector4 entityColor;
        Matrix4 modelMatrix;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {4, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 0},
                {5, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 1},
                {6, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 2},
                {7, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 3},
                {8, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(Vector4) * 4},
            };
        };
    };

    struct SkinnedVertex {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector4 tangent;
        Vector4 joints;
        Vector4 weights;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SkinnedVertex, position)},
                {1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SkinnedVertex, normal)},
                {2, VK_FORMAT_R32G32_SFLOAT, offsetof(SkinnedVertex, texCoords)},
                {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SkinnedVertex, tangent)},
                {4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SkinnedVertex, joints)},
                {5, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SkinnedVertex, weights)},
            };
        };
    };

    ComponentModel() = default;
    explicit ComponentModel(entt::registry& reg, entt::entity handle, const ModelPtr& model);
    virtual ~ComponentModel() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentModel);

    void setModel(ModelPtr value) {
        model = std::move(value);
    }

    [[nodiscard]] const ModelPtr& getModel() const {
        return model;
    }

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(model);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    ModelPtr model{nullptr};
};
} // namespace Engine
