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
        Matrix4 joints[Model::maxJoints];
        int count;
        char padding[60];
    };

    struct Cache {
        std::array<Matrix4, Model::maxJoints> inverseBindMat;
        std::array<Matrix4, Model::maxJoints> jointsLocalMat;
        std::array<Matrix4, Model::maxJoints> adjustmentsMat;
        int count{0};
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

    void setUboOffset(size_t value);
    [[nodiscard]] size_t getUboOffset() const {
        return uboOffset;
    }

    const Cache& getCache() const {
        return cache;
    }

    void setAdjustment(size_t joint, const Matrix4& value);

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(model);

protected:
    void patch(entt::registry& reg, entt::entity handle) override;

private:
    ModelPtr model{nullptr};
    Cache cache{};
    size_t uboOffset{0};
};
} // namespace Engine
