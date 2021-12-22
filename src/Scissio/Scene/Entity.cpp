#include "Entity.hpp"

#include "ComponentGrid.hpp"
#include "ComponentModel.hpp"
#include "ComponentParticleEmitter.hpp"

using namespace Scissio;

/*static constexpr auto ComponentsMask =
    (1 << ComponentModel::Type) | (1 << ComponentGrid::Type) | (1 << ComponentParticleEmitter::Type);

template <typename...> struct ComponentHelper {
    static ComponentPtr unpack(msgpack::object const& o, const ComponentType kind) {
        EXCEPTION("Cannot unpack component {} this should not happen!", kind);
    }

    static void pack(msgpack::packer<msgpack::sbuffer>& o, const ComponentPtr& component) {
        EXCEPTION("Cannot pack component {} this should not happen!", component->getType());
    }
};

template <typename C, typename... Cs> struct ComponentHelper<C, Cs...> {
    static ComponentPtr unpack(msgpack::object const& o, const ComponentType kind) {
        if (C::Type == kind) {
            auto ptr = std::make_shared<C>();
            o.convert<C>(*ptr);
            return ptr;
        } else {
            return ComponentHelper<Cs...>::unpack(o, kind);
        }
    }

    static void pack(msgpack::packer<msgpack::sbuffer>& o, const ComponentPtr& component) {
        if (C::Type == component->getType()) {
            o.pack(*std::dynamic_pointer_cast<C>(component));
        } else {
            ComponentHelper<Cs...>::pack(o, component);
        }
    }
};

using Helper = ComponentHelper<ComponentModel, ComponentGrid, ComponentParticleEmitter>;

MSGPACK_UNPACK_FUNC(ComponentPtr) {
    if (o.type != msgpack::type::ARRAY)
        throw msgpack::type_error();
    if (o.via.array.size != 2)
        throw msgpack::type_error();

    const auto kind = o.via.array.ptr[0].as<ComponentType>();

    if (o.via.array.ptr[1].type != msgpack::type::ARRAY)
        throw msgpack::type_error();

    v = Helper::unpack(o.via.array.ptr[1], kind);
    return o;
}

MSGPACK_PACK_FUNC(ComponentPtr) {
    o.pack_array(2);
    o.pack(v->getType());
    Helper::pack(o, v);
    return o;
}

MSGPACK_UNPACK_FUNC(Entity) {
    if (o.type != msgpack::type::ARRAY)
        throw msgpack::type_error();
    if (o.via.array.size != 3)
        throw msgpack::type_error();
    if (o.via.array.ptr[2].type != msgpack::type::ARRAY)
        throw msgpack::type_error();

    v = Entity(o.via.array.ptr[0].as<uint64_t>());
    v.updateTransform(o.via.array.ptr[1].as<Matrix4>());

    const auto& arr = o.via.array.ptr[2].via.array;
    for (size_t i = 0; i < arr.size; i++) {
        v.components.push_back(arr.ptr[i].as<ComponentPtr>());
        v.components.back()->setObject(v);
    }

    return o;
}

MSGPACK_PACK_FUNC(Entity) {
    o.pack_array(3);
    o.pack(v.id);
    o.pack(v.getTransform());

    auto count = 0;
    for (const auto& component : v.components) {
        if ((1 << component->getType()) & ComponentsMask) {
            count++;
        }
    }

    o.pack_array(count);
    for (const auto& component : v.components) {
        if ((1 << component->getType()) & ComponentsMask) {
            o.pack(component);
        }
    }

    return o;
}

MSGPACK_UNPACK_FUNC(EntityPtr) {
    v = std::make_shared<Entity>();
    o.convert<Entity>(*v);
    return o;
}

MSGPACK_PACK_FUNC(EntityPtr) {
    o.pack(*v);
    return o;
}*/
