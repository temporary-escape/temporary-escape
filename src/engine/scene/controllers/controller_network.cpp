#include "controller_network.hpp"
#include "../../network/peer.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerNetwork::ControllerNetwork(entt::registry& reg) : reg{reg} {
}

ControllerNetwork::~ControllerNetwork() = default;

void ControllerNetwork::update(const float delta) {
    (void)delta;
}

void ControllerNetwork::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

template <typename T> using ComponentReferences = std::array<std::tuple<entt::entity, const T*>, 64>;

template <typename Type>
static void sendComponents(Network::Peer& peer, const ComponentReferences<Type>& components, const size_t count,
                           const SyncOperation op) {
    static const auto id = EntityComponentIds::value<Type>;

    Network::Peer::Packer packer{peer, "MessageComponentSnapshot"};

    packer.pack_array(count);
    for (size_t i = 0; i < count; i++) {
        packer.pack_array(4);
        packer.pack(id);
        packer.pack(op);
        packer.pack(static_cast<uint32_t>(std::get<0>(components.at(i))));
        packer.pack(*std::get<1>(components.at(i)));
    }
}

template <typename View> static void packComponents(Network::Peer& peer, const View& view, const SyncOperation op) {
    using Iterable = typename View::iterable::value_type;
    using Type = typename std::remove_reference<typename std::tuple_element<1, Iterable>::type>::type;

    ComponentReferences<Type> components{};
    size_t count{0};

    for (auto&& [entity, component] : view.each()) {
        if (count >= components.size()) {
            sendComponents(peer, components, count, op);
            count = 0;
        }

        components[count++] = {entity, &component};
    }

    if (count > 0) {
        sendComponents(peer, components, count, op);
    }
}

template <typename T>
static void unpackComponent(entt::registry& reg, const entt::entity handle, const msgpack::object& obj,
                            const SyncOperation op) {
    if (op == SyncOperation::Emplace) {
        auto& component = reg.emplace<T>(handle);
        obj.convert(component);
        reg.patch<T>(handle);
    } else if (op == SyncOperation::Patch) {
        auto* component = reg.try_get<T>(handle);
        if (component) {
            obj.convert(*component);
            component->setDirty(true);
        }
    }
}

using UnpackerFunction = void (*)(entt::registry&, entt::entity, const msgpack::object&, const SyncOperation op);

static void unpackComponentId(entt::registry& reg, const uint32_t id, const entt::entity handle,
                              const msgpack::object& obj, const SyncOperation op) {
    static std::unordered_map<uint32_t, UnpackerFunction> unpackers = {
        {EntityComponentIds::value<ComponentTransform>, &unpackComponent<ComponentTransform>},
        {EntityComponentIds::value<ComponentModel>, &unpackComponent<ComponentModel>},
        {EntityComponentIds::value<ComponentRigidBody>, &unpackComponent<ComponentRigidBody>},
    };

    const auto found = unpackers.find(id);
    if (found != unpackers.end()) {
        found->second(reg, handle, obj, op);
    }
}

void ControllerNetwork::sendFullSnapshot(Network::Peer& peer) {
    packComponents(peer, reg.view<ComponentTransform>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentModel>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentRigidBody>(), SyncOperation::Emplace);
}

void ControllerNetwork::sendUpdate(Network::Peer& peer) {
    packComponents(peer, reg.view<ComponentTransform>(), SyncOperation::Patch);
}

void ControllerNetwork::resetUpdates() {
}

void ControllerNetwork::receiveUpdate(const msgpack::object& obj) {
    if (obj.type != msgpack::type::ARRAY) {
        EXCEPTION("Component snapshot is not an array");
    }

    const auto& arr = obj.via.array;
    logger.info("Received: {} components", arr.size);

    for (size_t i = 0; i < arr.size; i++) {
        if (arr.ptr[i].type != msgpack::type::ARRAY) {
            EXCEPTION("Component snapshot children: {} is not an array", i);
        }
        const auto& child = arr.ptr[i].via.array;
        if (child.ptr[0].type != msgpack::type::POSITIVE_INTEGER) {
            EXCEPTION("Component snapshot children: {} has a bad id", i);
        }
        if (child.ptr[2].type != msgpack::type::POSITIVE_INTEGER) {
            EXCEPTION("Component snapshot children: {} has a bad handle", i);
        }
        const auto id = child.ptr[0].as<uint32_t>();
        const auto op = child.ptr[1].as<SyncOperation>();
        const auto handle = static_cast<entt::entity>(child.ptr[2].as<uint32_t>());

        // Map the remote entity ID to a local one
        auto local = remoteToLocal.find(handle);
        if (local == remoteToLocal.end() && op == SyncOperation::Emplace) {
            local = remoteToLocal.emplace(handle, reg.create()).first;
        }

        if (local != remoteToLocal.end()) {
            unpackComponentId(reg, id, local->second, child.ptr[3], op);
        }
    }
}

void ControllerNetwork::onUpdateTransform(entt::registry& r, entt::entity handle) {
}