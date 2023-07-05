#pragma once

#include "../../assets/model.hpp"
#include "../component.hpp"

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

    ComponentModel() = default;
    explicit ComponentModel(entt::registry& reg, entt::entity handle);
    virtual ~ComponentModel() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentModel);

    void setModel(ModelPtr value) {
        model = std::move(value);
    }

    [[nodiscard]] const ModelPtr& getModel() const {
        return model;
    }

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(Component), model);

private:
    ModelPtr model{nullptr};
};
} // namespace Engine
