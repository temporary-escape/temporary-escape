#include "controller_network.hpp"
#include "../../network/peer.hpp"
#include <bitset>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerNetwork::ControllerNetwork(entt::registry& reg) : reg{reg} {
    registerComponent<ComponentTransform>();
    registerComponent<ComponentRigidBody>();
    reg.on_destroy<entt::entity>().connect<&ControllerNetwork::onDestroyEntity>(this);
}

ControllerNetwork::~ControllerNetwork() {
    unregisterComponent<ComponentTransform>();
    unregisterComponent<ComponentRigidBody>();
    reg.on_destroy<entt::entity>().disconnect<&ControllerNetwork::onDestroyEntity>(this);
}

void ControllerNetwork::update(const float delta) {
    (void)delta;
}

void ControllerNetwork::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

template <typename Type> static void postEmplaceComponent(const entt::entity handle, Type& component) {
    (void)handle;
    (void)component;
}

static void postEmplaceComponent(const entt::entity handle, ComponentRigidBody& component) {
    component.setup();
}

template <typename Type> static void postPatchComponent(const entt::entity handle, Type& component) {
    (void)handle;
    (void)component;
}

template <typename T> using ComponentReferences = std::array<std::tuple<entt::entity, const T*>, 64>;

template <typename Packer, typename Type>
static void packComponent(Packer& packer, entt::entity handle, const Type& component, const SyncOperation op) {
    static constexpr auto id = EntityComponentIds::value<Type>;
    packer.pack_array(4);
    packer.pack(id);
    packer.pack(op);
    packer.pack(static_cast<uint32_t>(handle));
    packer.pack(component);
}

template <typename Type>
static void sendComponents(Network::Peer& peer, const ComponentReferences<Type>& components, const size_t count,
                           const SyncOperation op) {
    Network::Peer::Packer packer{peer, "MessageComponentSnapshot"};

    packer.pack_array(count);
    for (size_t i = 0; i < count; i++) {
        packComponent(packer, std::get<0>(components.at(i)), *std::get<1>(components.at(i)), op);
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
        component.postUnpack(reg, handle);
        obj.convert(component);
        postEmplaceComponent(handle, component);
        component.setDirty(true);
    } else if (op == SyncOperation::Patch) {
        auto* component = reg.try_get<T>(handle);
        if (component) {
            obj.convert(*component);
            postPatchComponent(handle, *component);
            component->setDirty(true);
        }
    } else {
        logger.warn("Unknown sync operation for entity id: {}", static_cast<uint32_t>(handle));
    }
}

using UnpackerFunction = void (*)(entt::registry&, entt::entity, const msgpack::object&, const SyncOperation op);

static void unpackComponentId(entt::registry& reg, const uint32_t id, const entt::entity handle,
                              const msgpack::object& obj, const SyncOperation op) {
    static std::unordered_map<uint32_t, UnpackerFunction> unpackers = {
        {EntityComponentIds::value<ComponentTransform>, &unpackComponent<ComponentTransform>},
        {EntityComponentIds::value<ComponentModel>, &unpackComponent<ComponentModel>},
        {EntityComponentIds::value<ComponentRigidBody>, &unpackComponent<ComponentRigidBody>},
        {EntityComponentIds::value<ComponentIcon>, &unpackComponent<ComponentIcon>},
        {EntityComponentIds::value<ComponentLabel>, &unpackComponent<ComponentLabel>},
    };

    const auto found = unpackers.find(id);
    if (found != unpackers.end()) {
        found->second(reg, handle, obj, op);
    } else {
        logger.warn("Unmatched component id: {} entity id: {}", id, static_cast<uint32_t>(handle));
    }
}

void ControllerNetwork::sendFullSnapshot(Network::Peer& peer) {
    packComponents(peer, reg.view<ComponentTransform>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentModel>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentRigidBody>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentIcon>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentLabel>(), SyncOperation::Emplace);
}

void ControllerNetwork::sendUpdate(Network::Peer& peer) {
    size_t count{0};
    size_t arraySize{0};
    auto total{updatedComponentsCount};

    std::unique_ptr<Network::Peer::Packer> packer;
    const auto prepareNext = [&]() {
        if (count >= arraySize) {
            packer.reset();
            packer = std::make_unique<Network::Peer::Packer>(peer, "MessageComponentSnapshot");
            arraySize = std::min<size_t>(total, 64);
            count = 0;
            packer->pack_array(arraySize);
        }
        ++count;
        --total;
    };

    for (const auto& pair : updatedComponentsMap) {
        const auto handle = pair.first;
        if (pair.second & componentMaskId<ComponentTransform>()) {
            const auto& component = reg.get<ComponentTransform>(handle);
            prepareNext();
            packComponent(*packer, handle, component, SyncOperation::Patch);
        }
        if (pair.second & componentMaskId<ComponentRigidBody>()) {
            const auto& component = reg.get<ComponentRigidBody>(handle);
            prepareNext();
            packComponent(*packer, handle, component, SyncOperation::Patch);
        }
    }

    packer.reset();

    if (total != 0) {
        EXCEPTION("Something went wrong while packing scene updates, error: total != 0");
    }
}

void ControllerNetwork::resetUpdates() {
    updatedComponentsMap.clear();
    updatedComponentsCount = 0;
}

void ControllerNetwork::receiveUpdate(const msgpack::object& obj) {
    if (obj.type != msgpack::type::ARRAY) {
        EXCEPTION("Component snapshot is not an array");
    }

    const auto& arr = obj.via.array;
    // logger.debug("Received: {} components", arr.size);

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
        } else {
            logger.warn("Unmatched entity update id: {}", static_cast<uint32_t>(handle));
        }
    }
}

void ControllerNetwork::onDestroyEntity(entt::registry& r, entt::entity handle) {
    const auto it = updatedComponentsMap.find(handle);
    if (it != updatedComponentsMap.end()) {
        updatedComponentsCount -= std::bitset<64>{it->second}.count();
        updatedComponentsMap.erase(it);
    }
}
