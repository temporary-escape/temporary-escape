#pragma once

#include "ComponentCamera.hpp"
#include "ComponentCameraTop.hpp"
#include "ComponentCameraTurntable.hpp"
#include "ComponentCanvasImage.hpp"
#include "ComponentCanvasLines.hpp"
#include "ComponentDirectionalLight.hpp"
#include "ComponentCanvasLabel.hpp"
#include "ComponentModel.hpp"
#include "ComponentPlanet.hpp"
#include "ComponentSkybox.hpp"
#include "ComponentUserInput.hpp"
#include "Object.hpp"

#include <memory>
#include <stdexcept>

namespace Engine {
class Scene;

// clang-format off
using EntityComponentHelper = ComponentHelper<
    // DO NOT CHANGE THE ORDER! ADD NEW COMPONENTS AT THE BOTTOM!
    ComponentCamera,
    ComponentCameraTurntable,
    ComponentCameraTop,
    ComponentCanvasImage,
    ComponentCanvasLines,
    ComponentCanvasLabel,
    ComponentUserInput,
    ComponentSkybox,
    ComponentModel,
    ComponentDirectionalLight,
    ComponentPlanet
>;
// clang-format on

class ENGINE_API Entity : public Object, public std::enable_shared_from_this<Entity> {
public:
    struct ComponentReference {
        ComponentPtr ptr{nullptr};
        size_t type{0};
    };

    using ComponentsIterator = std::vector<ComponentReference>::iterator;

    explicit Entity(const uint64_t id = 0) : id(id) {
    }

    virtual ~Entity() = default;

    void setId(uint64_t id) {
        this->id = id;
    }

    [[nodiscard]] uint64_t getId() const {
        return id;
    }

    template <typename T, typename... Args> std::shared_ptr<T> addComponent(Args&&... args) {
        auto ptr = std::make_shared<T>(static_cast<Object&>(*this), std::forward<Args>(args)...);
        components.push_back(ComponentReference{ptr, EntityComponentHelper::getIndexOf<T>()});
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
    uint64_t id{0};
    std::vector<ComponentReference> components;
};

using EntityPtr = std::shared_ptr<Entity>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::Entity::ComponentReference> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Entity::ComponentReference& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();

            v.type = o.via.array.ptr[0].as<decltype(v.type)>();
            v.ptr = Engine::EntityComponentHelper::unpack(o.via.array.ptr[1], v.type);

            return o;
        }
    };

    template <> struct pack<Engine::Entity::ComponentReference> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o,
                                            Engine::Entity::ComponentReference const& v) const {

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

            o.pack_array(static_cast<uint32_t>(v->size()));
            for (const auto& ref : *v) {
                o.pack(ref);
            }

            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
