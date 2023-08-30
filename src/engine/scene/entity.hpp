#pragma once

#include "../library.hpp"
#include "../utils/msgpack_adaptors.hpp"
#include "../utils/msgpack_friend.hpp"
#include "components/component_camera.hpp"
#include "components/component_camera_orbital.hpp"
#include "components/component_camera_panning.hpp"
#include "components/component_debug.hpp"
#include "components/component_directional_light.hpp"
#include "components/component_grid.hpp"
#include "components/component_icon.hpp"
#include "components/component_label.hpp"
#include "components/component_lines.hpp"
#include "components/component_model.hpp"
#include "components/component_model_skinned.hpp"
#include "components/component_nebula.hpp"
#include "components/component_particle_emitter.hpp"
#include "components/component_planet.hpp"
#include "components/component_player.hpp"
#include "components/component_point_cloud.hpp"
#include "components/component_point_light.hpp"
#include "components/component_poly_shape.hpp"
#include "components/component_remote_handle.hpp"
#include "components/component_rigid_body.hpp"
#include "components/component_script.hpp"
#include "components/component_ship_control.hpp"
#include "components/component_skybox.hpp"
#include "components/component_star_flare.hpp"
#include "components/component_text.hpp"
#include "components/component_turret.hpp"
#include "components/component_world_text.hpp"
#include <entt/core/ident.hpp>
#include <entt/entity/view.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace Engine {
class ENGINE_API Scene;

using EntityComponentIds =
    entt::ident<TagDisabled, ComponentTransform, ComponentRemoteHandle, ComponentRigidBody, ComponentScript,
                ComponentCamera, ComponentCameraOrbital, ComponentCameraPanning, ComponentGrid, ComponentModel,
                ComponentModelSkinned, ComponentDirectionalLight, ComponentPointLight, ComponentPointCloud,
                ComponentLines, ComponentDebug, ComponentIcon, ComponentLabel, ComponentPolyShape, ComponentText,
                ComponentWorldText, ComponentPlanet, ComponentStarFlare, ComponentSkybox, ComponentNebula,
                ComponentTurret, ComponentShipControl>;

template <typename T> static inline constexpr uint64_t componentMaskId() {
    return 1ULL << EntityComponentIds::value<T>;
}

class ENGINE_API Entity {
public:
    using Handle = entt::entity;

    Entity() = default;
    explicit Entity(entt::registry& reg, entt::entity handle) : reg{&reg}, handle{handle} {
    }
    ~Entity() = default;

    void reset();
    void setDisabled(bool value);
    bool isDisabled() const;

    template <typename T> bool hasComponent() const {
        if (!reg) {
            EXCEPTION("Invalid entity");
        }

        return reg->template try_get<T>(handle) != nullptr;
    }

    template <typename T> T& getComponent() const {
        if (!reg) {
            EXCEPTION("Invalid entity");
        }

        if (!hasComponent<T>()) {
            EXCEPTION("Entity has no component of type: {}", typeid(T).name());
        }

        return reg->get<T>(handle);
    }

    template <typename T> T* tryGetComponent() const {
        if (!reg) {
            EXCEPTION("Invalid entity");
        }

        return reg->template try_get<T>(handle);
    }

    template <typename T, typename... Args> T& addComponent(Args&&... args) {
        if (!reg) {
            EXCEPTION("Invalid entity");
        }

        return reg->template emplace<T>(handle, *reg, handle, std::forward<Args>(args)...);
    }

    [[nodiscard]] entt::entity getHandle() const {
        return handle;
    }

    uint64_t getId() const {
        return static_cast<uint64_t>(getHandle());
    }

    operator bool() const {
        return reg && reg->valid(handle);
    }

    static void bind(Lua& lua);

private:
    entt::registry* reg{nullptr};
    entt::entity handle;
};

inline Vector4 entityColor(entt::entity handle) {
    const auto i = static_cast<uint32_t>(handle);
    const auto r = static_cast<float>((i & 0x000000FF) >> 0);
    const auto g = static_cast<float>((i & 0x0000FF00) >> 8);
    const auto b = static_cast<float>((i & 0x00FF0000) >> 16);
    const auto a = static_cast<float>((i & 0xFF000000) >> 24);
    return Vector4{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}

inline Vector4 entityColor(const ComponentTransform& transform) {
    const auto* parent = &transform;
    while (true) {
        if (!parent->getParent()) {
            return entityColor(parent->getHandle());
        }
        parent = parent->getParent();
    }
}

inline bool operator==(const std::optional<Entity>& a, std::optional<Entity>& b) {
    if (a && b) {
        return a->getHandle() == b->getHandle();
    }
    return !a && !b;
}

inline bool operator!=(const std::optional<Entity>& a, std::optional<Entity>& b) {
    return !(a == b);
}
} // namespace Engine
