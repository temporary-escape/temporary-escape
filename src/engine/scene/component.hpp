#pragma once

#include "../math/matrix.hpp"
#include "../utils/aligned.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/log.hpp"
#include "../utils/moveable_copyable.hpp"
#include "../vulkan/vulkan_renderer.hpp"
#include <entt/entity/registry.hpp>
#include <memory>
#include <msgpack.hpp>
#include <unordered_set>
#include <variant>

#define COMPONENT_DEFAULTS(ClassName)                                                                                  \
    NON_COPYABLE(ClassName)                                                                                            \
    MOVEABLE(ClassName)                                                                                                \
    static constexpr auto in_place_delete = true;

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API Component {
public:
    Component() = default;
    virtual ~Component() = default;

    [[nodiscard]] bool isDirty() const {
        return dirty;
    }

    void setDirty(const bool value) {
        dirty = value;
    }

private:
    bool dirty{false};
};

class ENGINE_API TagDisabled : public Component {
public:
    TagDisabled() = default;
    COMPONENT_DEFAULTS(TagDisabled);
};

/*class ENGINE_API Component;

class ENGINE_API AbstractComponentSystem {
public:
    virtual ~AbstractComponentSystem() = default;

    virtual void remove(Component& component) = 0;
    virtual void add(Component& component) = 0;
};

class ENGINE_API Component {
public:
    struct Flags {
        enum : uint64_t {
            None = 0,
            Syncable = 0x1 << 0,
        };
    };

    struct Reference {
        std::shared_ptr<Component> ptr{nullptr};
        size_t type{0};

        Component* operator->() const {
            return ptr.get();
        }

        const Component& operator*() const {
            return *ptr;
        }

        Component& operator*() {
            return *ptr;
        }

        bool operator<(const Reference& other) const {
            return (type < other.type);
        }
    };

    template <typename T, uint64_t F> struct Traits {
        using type = T;
        static constexpr inline uint64_t flags = F;
    };

    template <typename...> struct TraitsMapper;

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

    bool isDirty() const {
        return dirty;
    }

    void setDirty(const bool value) {
        dirty = value;
    }

    [[nodiscard]] uint64_t getRenderOrder() const {
        return renderOrder;
    }

    void setRenderOrder(uint64_t value) {
        renderOrder = value;
    }

private:
    Object* object{nullptr};
    AbstractComponentSystem* system{nullptr};
    bool dirty{false};
    uint64_t renderOrder{0};
};

inline Component::~Component() {
    if (system) {
        system->remove(*this);
    }
}

using ComponentPtr = std::shared_ptr<Component>;

template <typename T> class ENGINE_API ComponentSystem : public AbstractComponentSystem {
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

    [[nodiscard]] size_t size() const {
        return components.size();
    }

private:
    std::unordered_set<T*> components;
};

using ComponentSystemsMap = std::unordered_map<size_t, std::shared_ptr<AbstractComponentSystem>>;

template <> struct Component::TraitsMapper<> {
    static void generateComponentSystemsMap(ComponentSystemsMap& map, const size_t index) {
    }

    static uint64_t getFlags(const size_t idx) {
        (void)idx;
        return 0;
    }

    template <typename T> static constexpr size_t getIndexOf() {
        return 0;
    }

    static uint64_t getFlagsInternal(const size_t idx, const size_t counter) {
        (void)idx;
        (void)counter;
        return 0;
    }

    template <typename Stream>
    static void pack(msgpack::v1::packer<Stream>& o, const ComponentPtr& value, const size_t index,
                     const size_t current = 0) {
        throw msgpack::type_error();
    }

    static ComponentPtr unpack(msgpack::object const& o, const size_t index, const size_t current = 0) {
        throw msgpack::type_error();
    }

    template <typename D>
    static void getDelta(const ComponentPtr& value, const size_t index, D& delta, const size_t current = 0) {
        (void)value;
        (void)index;
        (void)delta;
        (void)current;
    }

    template <typename Tuple, typename Fn> static void forEach(Tuple& tuple, Fn& fn, const size_t current = 0) {
        (void)tuple;
        (void)fn;
        (void)current;
    }

    template <typename Tuple>
    static void insert(Tuple& tuple, const ComponentPtr& cmp, const size_t index, const size_t current = 0) {
        (void)tuple;
        (void)cmp;
        (void)index;
        (void)current;
    }
};

template <typename C, typename... Others> struct Component::TraitsMapper<C, Others...> {
    using Deltas = std::variant<typename C::type::Delta, typename Others::type::Delta...>;
    // using Tuple = std::tuple<std::shared_ptr<typename C::type>, std::shared_ptr<typename Others::type>...>;

    template <typename T> static constexpr size_t getIndexOf() {
        if constexpr (std::is_same<T, typename C::type>::value) {
            return 0;
        } else {
            return 1 + TraitsMapper<Others...>::template getIndexOf<T>();
        }
    }

    static uint64_t getFlagsInternal(const size_t idx, const size_t counter) {
        if (counter == idx) {
            return C::flags;
        } else {
            return TraitsMapper<Others...>::getFlagsInternal(idx, counter + 1);
        }
    }

    static uint64_t getFlags(const size_t idx) {
        return getFlagsInternal(idx, 0);
    }

    template <typename Stream>
    static void pack(msgpack::v1::packer<Stream>& o, const ComponentPtr& value, const size_t index,
                     const size_t current = 0) {
        if (index == current) {
            auto ptr = std::dynamic_pointer_cast<typename C::type>(value);
            o.pack(*ptr);
        } else {
            TraitsMapper<Others...>::template pack<Stream>(o, value, index, current + 1);
        }
    }

    static ComponentPtr unpack(msgpack::object const& o, const size_t index, const size_t current = 0) {
        if (index == current) {
            auto cmp = std::make_shared<typename C::type>();
            o.convert(*cmp);
            return cmp;
        } else {
            return TraitsMapper<Others...>::unpack(o, index, current + 1);
        }
    }

    template <typename D>
    static void getDelta(const ComponentPtr& value, const size_t index, D& delta, const size_t current = 0) {
        if (index == current) {
            auto ptr = std::dynamic_pointer_cast<typename C::type>(value);
            delta = ptr->getDelta();
        } else {
            return TraitsMapper<Others...>::template getDelta(value, index, delta, current + 1);
        }
    }

    static ComponentSystemsMap generateComponentSystemsMap() {
        ComponentSystemsMap map;
        generateComponentSystemsMap(map, 0);
        return map;
    }

    static void generateComponentSystemsMap(ComponentSystemsMap& map, const size_t index) {
        map.insert(std::make_pair(index, std::make_shared<ComponentSystem<typename C::type>>()));
        TraitsMapper<Others...>::generateComponentSystemsMap(map, index + 1);
    }
};*/
} // namespace Engine
