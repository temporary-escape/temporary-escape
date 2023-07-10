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
    explicit ComponentModel(entt::registry& reg, entt::entity handle, const ModelPtr& model);
    virtual ~ComponentModel() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentModel);

    void setModel(ModelPtr value) {
        model = std::move(value);
    }

    [[nodiscard]] const ModelPtr& getModel() const {
        return model;
    }

    void setStatic(const bool value) {
        flagStatic = value;
    }

    bool isStatic() const {
        return flagStatic;
    }

    static void bind(Lua& lua);

    MSGPACK_DEFINE_ARRAY(model, flagStatic);

private:
    ModelPtr model{nullptr};
    bool flagStatic{false};
};
} // namespace Engine
