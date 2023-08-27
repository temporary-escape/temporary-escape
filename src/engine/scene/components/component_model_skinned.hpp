#pragma once

#include "../../assets/model.hpp"
#include "../../math/matrix.hpp"
#include "../component.hpp"

namespace Engine {
class ENGINE_API ComponentModelSkinned : public Component {
public:
    struct Vertex {
        Vector3 position;
        Vector3 normal;
        Vector2 texCoords;
        Vector4 tangent;
        Vector4 joints;
        Vector4 weights;

        static VulkanVertexLayoutMap getLayout() {
            return {
                {0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)},
                {1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},
                {2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoords)},
                {3, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)},
                {4, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, joints)},
                {5, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weights)},
            };
        };
    };

    struct Armature {
        Matrix4 joints[16];
        int count;
        char padding[60];
    };

    static_assert(sizeof(Armature) % 64 == 0);

    ComponentModelSkinned() = default;
    explicit ComponentModelSkinned(entt::registry& reg, entt::entity handle, const ModelPtr& model);
    virtual ~ComponentModelSkinned() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentModelSkinned);

    void setModel(ModelPtr value);

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
