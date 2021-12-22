#pragma once

#include "../Graphics/Shader.hpp"
#include "../Utils/Log.hpp"
#include "Object.hpp"

#include <memory>
#include <unordered_set>

namespace Scissio {
class SCISSIO_API Component;

class SCISSIO_API AbstractComponentSystem {
public:
    virtual ~AbstractComponentSystem() = default;

    virtual void remove(Component& component) = 0;
    virtual void add(Component& component) = 0;
};

using ComponentType = uint32_t;

class SCISSIO_API Component {
public:
    Component() = default;

    explicit Component(const ComponentType type) : type(type) {
    }

    explicit Component(const ComponentType type, Object& object) : type(type), object(&object) {
    }
    virtual ~Component() = 0;

    // This function can only be used by msgpack!
    void setObject(Object& object) {
        this->object = &object;
    }

    Object& getObject() const {
        assert(!!object);
        return *object;
    }

    void setSystem(AbstractComponentSystem* system) {
        assert(!!system);
        this->system = system;
    }

    ComponentType getType() const {
        return type;
    }

protected:
    ComponentType type{0};

private:
    Object* object{nullptr};
    AbstractComponentSystem* system{nullptr};
};

inline Component::~Component() {
    if (system) {
        system->remove(*this);
    }
}

using ComponentPtr = std::shared_ptr<Component>;

template <typename T> class SCISSIO_API ComponentSystem : public AbstractComponentSystem {
public:
    ComponentSystem() = default;
    virtual ~ComponentSystem() {
        if (!components.empty()) {
            Log::w("ComponentSystem", "Component system destroyed with {} components attached", components.size());
        }
    }

    void remove(Component& component) override {
        auto ptr = reinterpret_cast<T*>(&component);
        auto it = components.find(ptr);
        if (it != components.end()) {
            components.erase(it);
        }
    }

    void add(Component& component) override {
        if (component.getType() != T::Type) {
            return;
        }
        auto ptr = reinterpret_cast<T*>(&component);
        if (ptr) {
            components.insert(ptr);
            ptr->setSystem(this);
        }
    }

    typename std::unordered_set<T*>::iterator begin() {
        return components.begin();
    }

    typename std::unordered_set<T*>::iterator end() {
        return components.end();
    }

private:
    std::unordered_set<T*> components;
};

template <typename...> struct ComponentHelper {
    static ComponentPtr unpack(msgpack::v1::object const& o, const ComponentType kind) {
        return nullptr;
    }

    template <typename Stream> static void pack(msgpack::v1::packer<Stream>& o, const ComponentPtr& component) {
    }

    static bool isPackable(const ComponentPtr& component) {
        return false;
    }
};

template <typename C, typename... Cs> struct ComponentHelper<C, Cs...> {
    static ComponentPtr unpack(msgpack::v1::object const& o, const ComponentType kind) {
        if (C::Type == kind) {
            auto ptr = std::make_shared<C>();
            o.convert<C>(*ptr);
            return ptr;
        } else {
            return ComponentHelper<Cs...>::unpack(o, kind);
        }
    }

    template <typename Stream> static void pack(msgpack::v1::packer<Stream>& o, const ComponentPtr& component) {
        if (C::Type == component->getType()) {
            o.pack(*std::dynamic_pointer_cast<C>(component));
        } else {
            ComponentHelper<Cs...>::template pack<Stream>(o, component);
        }
    }

    static bool isPackable(const ComponentPtr& component) {
        if (C::Type == component->getType()) {
            return true;
        } else {
            return ComponentHelper<Cs...>::isPackable(component);
        }
    }
};
} // namespace Scissio
