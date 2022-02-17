#pragma once

#include "ComponentCamera.hpp"
#include "ComponentCameraTop.hpp"
#include "ComponentCameraTurntable.hpp"
#include "ComponentCanvasImage.hpp"
#include "ComponentCanvasLabel.hpp"
#include "ComponentCanvasLines.hpp"
#include "ComponentDirectionalLight.hpp"
#include "ComponentGrid.hpp"
#include "ComponentModel.hpp"
#include "ComponentParticleEmitter.hpp"
#include "ComponentPlanet.hpp"
#include "ComponentShipControl.hpp"
#include "ComponentSkybox.hpp"
#include "ComponentTurret.hpp"
#include "ComponentUserInput.hpp"
#include "Object.hpp"

#include <memory>
#include <stdexcept>

namespace Engine {
class Scene;

// clang-format off
using EntityComponentHelper = Component::TraitsMapper<
    // DO NOT CHANGE THE ORDER! ADD NEW COMPONENTS AT THE BOTTOM!
    Component::Traits<ComponentCamera,
                      Component::Flags::None>,
    Component::Traits<ComponentCameraTurntable,
                      Component::Flags::None>,
    Component::Traits<ComponentCameraTop,
                      Component::Flags::None>,
    Component::Traits<ComponentCanvasImage,
                      Component::Flags::None>,
    Component::Traits<ComponentCanvasLines,
                      Component::Flags::None>,
    Component::Traits<ComponentCanvasLabel,
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
                      Component::Flags::Syncable>
>;
// clang-format on

class ENGINE_API EntityProxy {
public:
    virtual ~EntityProxy() = 0;
};

inline EntityProxy::~EntityProxy() = default;

class ENGINE_API Entity : public EntityProxy, public Object, public std::enable_shared_from_this<Entity> {
public:
    struct Delta {
        uint64_t id{0};
        Matrix4 transform;
        std::vector<typename EntityComponentHelper::Deltas> components;

        MSGPACK_DEFINE_ARRAY(id, transform, components);
    };

    using ComponentsIterator = std::vector<Component::Reference>::iterator;

    explicit Entity() : id(0) {
    }

    void setParent(const std::shared_ptr<Entity>& value);

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
        auto ptr = std::make_shared<T>(static_cast<Object&>(*this), std::forward<Args>(args)...);
        components.push_back(Component::Reference{ptr, EntityComponentHelper::getIndexOf<T>()});
        return ptr;
    }

    template <typename T> std::shared_ptr<T> getComponent() {
        for (const auto& component : components) {
            if (component.type == EntityComponentHelper::getIndexOf<T>()) {
                return std::dynamic_pointer_cast<T>(component.ptr);
            }
        }
        throw std::out_of_range("Component does not exist");
    }

    template <typename T> std::optional<std::shared_ptr<T>> getComponentOpt() {
        for (const auto& component : components) {
            if (component.type == EntityComponentHelper::getIndexOf<T>()) {
                return std::dynamic_pointer_cast<T>(component.ptr);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] ComponentsIterator begin() {
        return components.begin();
    }

    [[nodiscard]] ComponentsIterator end() {
        return components.end();
    }

    [[nodiscard]] size_t size() const {
        return components.size();
    }

    MSGPACK_FRIEND(std::shared_ptr<Entity>);

private:
    void addChild(const std::shared_ptr<Entity>& child) {
        children.push_back(child);
    }

    void removeChildInternal(const Entity* child) {
        children.erase(std::remove_if(children.begin(), children.end(), [&](auto& ptr) { return ptr.get() == child; }),
                       children.end());
    }

    uint64_t id{0};
    std::vector<Component::Reference> components;
    std::vector<std::shared_ptr<Entity>> children;
};

using EntityPtr = std::shared_ptr<Entity>;
using EntityProxyPtr = std::shared_ptr<EntityProxy>;
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

    template <> struct pack<Engine::Component::Reference> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Component::Reference const& v) const {

            o.pack_array(2);
            o.pack(v.type);
            Engine::EntityComponentHelper::pack<Stream>(o, v.ptr, v.type);

            return o;
        }
    };

    template <> struct convert<Engine::EntityPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::EntityPtr& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            if (o.via.array.ptr[2].type != msgpack::type::ARRAY)
                throw msgpack::type_error();

            v = std::make_shared<Engine::Entity>();
            o.via.array.ptr[0].convert(v->id);
            v->updateTransform(o.via.array.ptr[1].as<Engine::Matrix4>());

            const auto& arr = o.via.array.ptr[2];
            for (size_t i = 0; i < arr.via.array.size; i++) {
                v->components.emplace_back();
                arr.via.array.ptr[i].convert(v->components.back());
                v->components.back().ptr->setObject(*v);
            }

            return o;
        }
    };

    template <> struct pack<Engine::EntityPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::EntityPtr const& v) const {
            o.pack_array(3);
            o.pack(v->getId());
            o.pack(v->getTransform());

            // Calculate what can be synced
            size_t count = 0;
            for (const auto& ref : *v) {
                if (Engine::EntityComponentHelper::getFlags(ref.type) & Engine::Component::Flags::Syncable) {
                    count++;
                }
            }

            o.pack_array(static_cast<uint32_t>(count));
            for (const auto& ref : *v) {
                if (Engine::EntityComponentHelper::getFlags(ref.type) & Engine::Component::Flags::Syncable) {
                    o.pack(ref);
                }
            }

            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
