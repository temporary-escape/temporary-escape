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

class SCISSIO_API Component {
public:
    Component() = default;

    explicit Component(Object& object) : object(&object) {
    }

    virtual ~Component() = 0;

    void setObject(Object& object) { // Used by msgpack only
        this->object = &object;
    }

    [[nodiscard]] Object& getObject() const {
        assert(!!object);
        return *object;
    }

    void setSystem(AbstractComponentSystem* system) {
        assert(!!system);
        this->system = system;
    }

    void clearSystem() {
        this->system = nullptr;
    }

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
            component.clearSystem();
        }
    }

    void add(Component& component) override {
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

using ComponentSystemsMap = std::unordered_map<size_t, std::shared_ptr<AbstractComponentSystem>>;

template <typename...> struct ComponentHelper;

template <> struct ComponentHelper<> {
    static void generateComponentSystemsMap(ComponentSystemsMap& map, const size_t index) {
    }

    template <typename Stream>
    static void pack(msgpack::v1::packer<Stream>& o, const ComponentPtr& value, const size_t index,
                     const size_t current = 0) {
        throw msgpack::type_error();
    }

    static ComponentPtr unpack(msgpack::object const& o, const size_t index, const size_t current = 0) {
        throw msgpack::type_error();
    }
};

template <typename ComponentT, typename... ComponentsT> struct ComponentHelper<ComponentT, ComponentsT...> {
    template <typename T> static constexpr size_t getIndexOf() {
        if constexpr (std::is_same<T, ComponentT>::value) {
            return 0;
        } else {
            return 1 + ComponentHelper<ComponentsT...>::template getIndexOf<T>();
        }
    }

    template <typename Stream>
    static void pack(msgpack::v1::packer<Stream>& o, const ComponentPtr& value, const size_t index,
                     const size_t current = 0) {
        if (index == current) {
            auto ptr = std::dynamic_pointer_cast<ComponentT>(value);
            o.pack(*ptr);
        } else {
            ComponentHelper<ComponentsT...>::template pack<Stream>(o, value, index, current + 1);
        }
    }

    static ComponentPtr unpack(msgpack::object const& o, const size_t index, const size_t current = 0) {
        if (index == current) {
            auto cmp = std::make_shared<ComponentT>();
            o.convert(*cmp);
            return cmp;
        } else {
            return ComponentHelper<ComponentsT...>::unpack(o, index, current + 1);
        }
    }

    static ComponentSystemsMap generateComponentSystemsMap() {
        ComponentSystemsMap map;
        generateComponentSystemsMap(map, 0);
        return map;
    }

    static void generateComponentSystemsMap(ComponentSystemsMap& map, const size_t index) {
        map.insert(std::make_pair(index, std::make_shared<ComponentSystem<ComponentT>>()));
        ComponentHelper<ComponentsT...>::generateComponentSystemsMap(map, index + 1);
    }
};
} // namespace Scissio
