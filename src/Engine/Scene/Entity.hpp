#pragma once

#include "../Library.hpp"
#include "../Utils/MsgpackAdaptors.hpp"
#include "../Utils/MsgpackFriend.hpp"
#include "Components/ComponentCamera.hpp"
#include "Components/ComponentCameraOrbital.hpp"
#include "Components/ComponentCameraPanning.hpp"
#include "Components/ComponentDebug.hpp"
#include "Components/ComponentDirectionalLight.hpp"
#include "Components/ComponentGrid.hpp"
#include "Components/ComponentIcon.hpp"
#include "Components/ComponentLabel.hpp"
#include "Components/ComponentLines.hpp"
#include "Components/ComponentModel.hpp"
#include "Components/ComponentModelSkinned.hpp"
#include "Components/ComponentNebula.hpp"
#include "Components/ComponentParticleEmitter.hpp"
#include "Components/ComponentPlanet.hpp"
#include "Components/ComponentPlayer.hpp"
#include "Components/ComponentPointCloud.hpp"
#include "Components/ComponentPointLight.hpp"
#include "Components/ComponentPolyShape.hpp"
#include "Components/ComponentRigidBody.hpp"
#include "Components/ComponentScript.hpp"
#include "Components/ComponentShipControl.hpp"
#include "Components/ComponentSkybox.hpp"
#include "Components/ComponentStarFlare.hpp"
#include "Components/ComponentText.hpp"
#include "Components/ComponentTurret.hpp"
#include "Components/ComponentWorldText.hpp"
#include <entt/core/ident.hpp>
#include <entt/entity/view.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace Engine {
class ENGINE_API Scene;

using EntityComponentIds =
    entt::ident<TagDisabled, ComponentTransform, ComponentRigidBody, ComponentScript, ComponentCamera,
                ComponentCameraOrbital, ComponentCameraPanning, ComponentGrid, ComponentModel, ComponentModelSkinned,
                ComponentDirectionalLight, ComponentPointLight, ComponentPointCloud, ComponentLines, ComponentDebug,
                ComponentIcon, ComponentLabel, ComponentPolyShape, ComponentText, ComponentWorldText, ComponentPlanet,
                ComponentStarFlare, ComponentSkybox, ComponentNebula, ComponentTurret, ComponentShipControl>;

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

        return reg->template emplace<T>(handle, handle, std::forward<Args>(args)...);
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
            return entityColor(parent->getEntity());
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
