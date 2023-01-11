#pragma once

#include "../library.hpp"
#include "../utils/msgpack_adaptors.hpp"
#include "../utils/msgpack_friend.hpp"
#include "component_camera.hpp"
#include "component_debug.hpp"
#include "component_directional_light.hpp"
#include "component_grid.hpp"
#include "component_icon_point_cloud.hpp"
#include "component_label.hpp"
#include "component_lines.hpp"
#include "component_model.hpp"
#include "component_particle_emitter.hpp"
#include "component_planet.hpp"
#include "component_player.hpp"
#include "component_point_cloud.hpp"
#include "component_poly_shape.hpp"
#include "component_script.hpp"
#include "component_ship_control.hpp"
#include "component_text.hpp"
#include "component_turret.hpp"
#include "component_user_input.hpp"
#include <entt/core/ident.hpp>
#include <entt/entity/view.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace Engine {
class ENGINE_API Scene;

using EntityComponentIds =
    entt::identifier<ComponentTransform, ComponentCamera, ComponentGrid, ComponentModel, ComponentDirectionalLight,
                     ComponentUserInput, ComponentPointCloud, ComponentIconPointCloud, ComponentLines, ComponentDebug>;

class ENGINE_API Entity {
public:
    explicit Entity(entt::registry& reg) : reg{&reg}, handle{reg.create()} {
    }
    ~Entity() = default;

    void destroy();

    void setVisible(bool value) {
        visible = value;
    }

    [[nodiscard]] bool isVisible() const {
        return visible;
    }

    template <typename T> bool hasComponent() {
        const auto index = EntityComponentIds::type<T>;
        return (mask & (1 << index)) != 0;
    }

    template <typename T> T& getComponent() {
        if (!reg) {
            EXCEPTION("Invalid entity");
        }

        if (!hasComponent<T>()) {
            EXCEPTION("Entity does not contain component of this type");
        }

        return reg->get<T>(handle);
    }

    template <typename T, typename... Args> T& addComponent(Args&&... args) {
        if (!reg) {
            EXCEPTION("Invalid entity");
        }

        if (hasComponent<T>()) {
            EXCEPTION("Entity already contains component of this type");
        }

        auto& cmp = reg->template emplace<T>(handle, std::forward<Args>(args)...);
        const auto index = EntityComponentIds::type<T>;
        mask |= 1 << index;
        return cmp;
    }

private:
    entt::registry* reg{nullptr};
    entt::entity handle;
    bool visible{true};
    uint64_t mask{0};
};

using EntityPtr = std::shared_ptr<Entity>;
using EntityWeakPtr = std::weak_ptr<Entity>;

/*
// clang-format off
using EntityComponentHelper = Component::TraitsMapper<
    // DO NOT CHANGE THE ORDER! ADD NEW COMPONENTS AT THE BOTTOM!
    Component::Traits<ComponentDebug,
                      Component::Flags::None>,
    Component::Traits<ComponentScript,
                      Component::Flags::None>,
    Component::Traits<ComponentCamera,
                      Component::Flags::None>,
    Component::Traits<ComponentPointCloud,
                      Component::Flags::None>,
    Component::Traits<ComponentIconPointCloud,
                      Component::Flags::None>,
    Component::Traits<ComponentLines,
                      Component::Flags::None>,
    Component::Traits<ComponentPolyShape,
                      Component::Flags::None>,
    Component::Traits<ComponentWireframe,
                      Component::Flags::None>,
    Component::Traits<ComponentText,
                      Component::Flags::None>,
    Component::Traits<ComponentUserInput,
                      Component::Flags::None>,
    Component::Traits<ComponentSkybox,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentModel,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentGrid,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentTurret,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentParticleEmitter,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentDirectionalLight,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentShipControl,
                      Component::Flags::None>,
    Component::Traits<ComponentPlanet,
                      Component::Flags::None>,
    Component::Traits<ComponentLabel,
                      Component::Flags::Syncable>,
    Component::Traits<ComponentPlayer,
                      Component::Flags::Syncable>
>;
// clang-format on

class ENGINE_API Entity;

class ENGINE_API EntityProxy {
public:
    virtual ~EntityProxy() = 0;
};

inline EntityProxy::~EntityProxy() = default;

using EntityPtr = std::shared_ptr<Entity>;
using EntityProxyPtr = std::shared_ptr<EntityProxy>;
using EntityWeakPtr = std::weak_ptr<Entity>;

class ENGINE_API Entity : public EntityProxy, public Object, public std::enable_shared_from_this<Entity> {
public:
    struct Delta {
        uint64_t id{0};
        Matrix4 transform;
        bool visible{true};
        std::vector<typename EntityComponentHelper::Deltas> components;

        MSGPACK_DEFINE_ARRAY(id, transform, visible, components);
    };

    // using ComponentsIterator = std::vector<Component::Reference>::iterator;

    explicit Entity() : id(0) {
    }

    void setParent(const std::shared_ptr<Entity>& value);

    std::shared_ptr<Entity> getParent() {
        if (!getParentObject()) {
            return nullptr;
        }
        auto ptr = dynamic_cast<Entity*>(getParentObject());
        return ptr->shared_from_this();
    }

    uint64_t getParentId() const {
        return parentId;
    }

    void removeChild(const std::shared_ptr<Entity>& value);

    void clearChildren();

    virtual ~Entity();

    Delta getDelta() const;

    void applyDelta(const Delta& delta);

    void setId(const uint64_t value) {
        id = value;
    }

    const std::vector<std::shared_ptr<Entity>>& getChildren() const {
        return children;
    }

    [[nodiscard]] uint64_t getId() const {
        return id;
    }

    template <typename T, typename... Args> std::shared_ptr<T> addComponent(Args&&... args) {
        if (hasComponent<T>()) {
            EXCEPTION("Component '{}' already attached to the entity", typeid(T).name());
        }
        auto ptr = std::make_shared<T>(static_cast<Object&>(*this), std::forward<Args>(args)...);
        components.push_back(Component::Reference{ptr, EntityComponentHelper::getIndexOf<T>()});
        componentMask |= (1 << components.back().type);
        std::sort(components.begin(), components.end());
        return ptr;
    }

    template <typename T> bool hasComponent() const {
        return componentMask & (1 << EntityComponentHelper::getIndexOf<T>());
    }

    template <typename T> std::shared_ptr<T> findComponent() {
        if (!hasComponent<T>()) {
            return nullptr;
        }

        for (const auto& ref : components) {
            if (ref.type == EntityComponentHelper::getIndexOf<T>()) {
                return std::dynamic_pointer_cast<T>(ref.ptr);
            }
        }

        EXCEPTION("Component mask failure during getComponents() function, this should never happen!");
    }

    template <typename T> std::shared_ptr<T> getComponent() {
        if (!hasComponent<T>()) {
            throw std::out_of_range("Component not found");
        }

        for (const auto& ref : components) {
            if (ref.type == EntityComponentHelper::getIndexOf<T>()) {
                return std::dynamic_pointer_cast<T>(ref.ptr);
            }
        }

        EXCEPTION("Component mask failure during getComponents() function, this should never happen!");
    }

    const std::vector<Component::Reference>& getComponents() const {
        return components;
    }

    MSGPACK_FRIEND(std::shared_ptr<Entity>);
    MSGPACK_FRIEND(std::shared_ptr<EntityProxy>);

private:
    void updateComponents() {

        std::sort(components.begin(), components.end());
        componentMask = 0;
        for (const auto& ref : components) {
            componentMask &= ref.type;
        }
    }

    void addChild(const std::shared_ptr<Entity>& child) {
        children.push_back(child);
    }

    void removeChildInternal(const Entity* child) {
        children.erase(std::remove_if(children.begin(), children.end(), [&](auto& ptr) { return ptr.get() == child; }),
                       children.end());
    }

    uint64_t id{0};
    std::vector<Component::Reference> components;
    std::vector<EntityPtr> children;
    uint64_t parentId{0};
    uint64_t componentMask{0};
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::Component::Reference> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Component::Reference& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();

            v.type = o.via.array.ptr[0].as<decltype(v.type)>();
            v.ptr = Engine::EntityComponentHelper::unpack(o.via.array.ptr[1], v.type);

            return o;
        }
    };

    template <> struct convert<Engine::EntityProxyPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::EntityProxyPtr& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();

            v = std::make_shared<Engine::Entity>();
            auto e = std::dynamic_pointer_cast<Engine::Entity>(v);
            o.via.array.ptr[0].convert(e->id);
            e->updateTransform(o.via.array.ptr[1].as<Engine::Matrix4>());

            const auto& components = o.via.array.ptr[2];
            if (o.via.array.ptr[2].type != msgpack::type::ARRAY) {
                throw msgpack::type_error();
            }

            o.via.array.ptr[2].convert(e->components);
            for (const auto& ref : e->components) {
                ref.ptr->setObject(*e);
                e->componentMask |= (1 << ref.type);
            }

            if (o.via.array.ptr[3].type == msgpack::type::POSITIVE_INTEGER) {
                o.via.array.ptr[3].convert(e->parentId);
            }

            return o;
        }
    };

    template <typename Stream>
    inline void packComponent(msgpack::packer<Stream>& o, Engine::EntityPtr const& v, const uint64_t mask) {
        // Calculate what can be synced
        size_t count = 0;
        for (const auto& ref : v->getComponents()) {
            if (Engine::EntityComponentHelper::getFlags(ref.type) & mask) {
                count++;
            }
        }

        o.pack_array(static_cast<uint32_t>(count));

        for (const auto& ref : v->getComponents()) {
            if (Engine::EntityComponentHelper::getFlags(ref.type) & mask) {
                o.pack_array(2);
                o.pack(ref.type);
                Engine::EntityComponentHelper::pack(o, ref.ptr, ref.type);
            }
        }
    }

    template <> struct pack<Engine::EntityProxyPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::EntityProxyPtr const& v) const {
            auto e = std::dynamic_pointer_cast<Engine::Entity>(v);

            o.pack_array(4);
            o.pack(e->getId());
            o.pack(e->getTransform());

            packComponent(o, e, Engine::Component::Flags::Syncable);

            if (auto parent = e->getParent()) {
                o.pack(parent->getId());
            } else {
                o.pack_nil();
            }

            return o;
        }
    };
    } // namespace adaptor
}*/
} // namespace Engine
