#pragma once

#include "ComponentModel.hpp"
#include "ComponentSkybox.hpp"
#include "Object.hpp"

#include <memory>
#include <stdexcept>

namespace Scissio {
class Scene;

class SCISSIO_API Entity : public Object, public std::enable_shared_from_this<Entity> {
public:
    explicit Entity(const uint64_t id = 0) : id(id) {
    }

    virtual ~Entity() = default;

    [[nodiscard]] uint64_t getId() const {
        return id;
    }

    template <typename T, typename... Args> std::shared_ptr<T> addComponent(Args&&... args) {
        auto ptr = std::make_shared<T>(static_cast<Object&>(*this), std::forward<Args>(args)...);
        components.push_back(ptr);
        return ptr;
    }

    template <typename T> std::shared_ptr<T> getComponent() {
        for (auto& component : components) {
            if (component->getType() == T::Type) {
                return std::dynamic_pointer_cast<T>(component);
            }
        }
        throw std::out_of_range("Component does not exist");
    }

    [[nodiscard]] const std::vector<ComponentPtr>& getComponents() const {
        return components;
    }

    friend Scene;
    // MSGPACK_FRIEND(Entity);

private:
    uint64_t id{0};
    std::vector<ComponentPtr> components;

public:
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE(Object), id, components);
};

using EntityPtr = std::shared_ptr<Entity>;

struct EntityClientView {
    EntityPtr ptr;
};

using ComponentHelperImpl = ComponentHelper<ComponentSkybox, ComponentModel>;
} // namespace Scissio

namespace msgpack::v3::adaptor {
template <> struct convert<Scissio::ComponentPtr> {
    msgpack::v1::object const& operator()(msgpack::v1::object const& o, Scissio::ComponentPtr& v) const {
        return o;
    }
};

template <> struct pack<Scissio::ComponentPtr> {
    template <typename Stream>
    msgpack::v1::packer<Stream>& operator()(msgpack::v1::packer<Stream>& o, Scissio::ComponentPtr const& v) const {
        return o;
    }
};

template <> struct convert<Scissio::EntityClientView> {
    msgpack::v1::object const& operator()(msgpack::v1::object const& o, Scissio::EntityClientView& v) const {
        return o;
    }
};

template <> struct pack<Scissio::EntityClientView> {
    template <typename Stream>
    msgpack::v1::packer<Stream>& operator()(msgpack::v1::packer<Stream>& o, Scissio::EntityClientView const& v) const {
        o.pack_array(3);
        o.pack(v.ptr->getTransform());
        o.pack(v.ptr->getId());

        size_t count = 0;
        for (const auto& component : v.ptr->getComponents()) {
            if (Scissio::ComponentHelperImpl::isPackable(component)) {
                ++count;
            }
        }

        o.pack_array(count);
        for (const auto& component : v.ptr->getComponents()) {
            Scissio::ComponentHelperImpl::pack<Stream>(o, component);
        }

        return o;
    }
};
} // namespace msgpack::v3::adaptor
