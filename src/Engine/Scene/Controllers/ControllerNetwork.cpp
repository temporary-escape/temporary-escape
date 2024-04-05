#include "ControllerNetwork.hpp"
#include "../../Network/NetworkStream.hpp"
#include "../../Server/Messages.hpp"
#include "../Scene.hpp"
#include <bitset>
#include <btBulletDynamicsCommon.h>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ControllerNetwork::ControllerNetwork(Scene& scene, entt::registry& reg) : scene{scene}, reg{reg} {
    registerComponent<ComponentTransform>();
    registerComponent<ComponentRigidBody>();
    registerComponent<ComponentTurret>();
    registerComponent<ComponentShipControl>();
    reg.on_destroy<entt::entity>().connect<&ControllerNetwork::onDestroyEntity>(this);
}

ControllerNetwork::~ControllerNetwork() {
    unregisterComponent<ComponentTransform>();
    unregisterComponent<ComponentRigidBody>();
    unregisterComponent<ComponentTurret>();
    unregisterComponent<ComponentShipControl>();
    reg.on_destroy<entt::entity>().disconnect<&ControllerNetwork::onDestroyEntity>(this);
}

void ControllerNetwork::update(const float delta) {
    (void)delta;
}

void ControllerNetwork::recalculate(VulkanRenderer& vulkan) {
    (void)vulkan;
}

template <typename Type>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle, Type& component) {
    (void)remoteId;
    (void)handle;
    (void)component;
    scene.setDirty(component);
}

template <>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle,
                                             ComponentTransform& component) {
    (void)handle;

    // reg.emplace<ComponentRemoteHandle>(handle, reg, handle, remoteId);

    while (true) {
        const auto match = std::find_if(transformChildParentMap.begin(),
                                        transformChildParentMap.end(),
                                        [&](const auto& el) { return el.parentId == remoteId; });
        if (match != transformChildParentMap.end()) {
            const auto transform = reg.try_get<ComponentTransform>(match->child);
            if (transform) {
                transform->setParent(&component);
            }
            logger.warn("Matched parent transform id: {} to child: {}",
                        static_cast<uint64_t>(handle),
                        static_cast<uint64_t>(match->child));
            transformChildParentMap.erase(match);
        } else {
            break;
        }
    }

    if (component.getParentId() != ComponentTransform::NullParentId) {
        logger.warn("Emplace component transform with parent");
        auto local = remoteToLocal.find(static_cast<entt::entity>(component.getParentId()));
        if (local != remoteToLocal.end()) {
            const auto transform = reg.try_get<ComponentTransform>(local->second);
            if (transform) {
                component.setParent(transform);
            } else {
                logger.warn("Failed to set parent transform id: {} for child: {}",
                            component.getParentId(),
                            static_cast<uint64_t>(handle));
            }
        } else {
            logger.warn("Failed to find parent transform id: {} for child: {}",
                        component.getParentId(),
                        static_cast<uint64_t>(handle));

            transformChildParentMap.push_back({component.getParentId(), handle});
        }
    }

    scene.setDirty(component);
}

/*template <>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle,
                                             ComponentRigidBody& component) {
    (void)remoteId;
    (void)handle;
    // component.setup();
    // component.setDirty(true);
}*/

template <>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle,
                                             ComponentGrid& component) {
    (void)remoteId;
    (void)handle;
    component.setDirty();
    scene.setDirty(component);
}

template <>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle,
                                             ComponentModel& component) {
    (void)remoteId;
    (void)handle;
    scene.setDirty(component);
}

template <>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle,
                                             ComponentModelSkinned& component) {
    (void)remoteId;
    (void)handle;
    component.setModel(component.getModel());
    scene.setDirty(component);
}

template <>
void ControllerNetwork::postEmplaceComponent(const uint64_t remoteId, const entt::entity handle,
                                             ComponentShipControl& component) {
    (void)remoteId;
    (void)handle;
    component.setActive(false);
    scene.setDirty(component);
}

template <typename Type>
void ControllerNetwork::postPatchComponent(const uint64_t remoteId, const entt::entity handle, Type& component) {
    (void)remoteId;
    (void)handle;
    scene.setDirty(component);
}

template <typename Packer, typename Type>
void ControllerNetwork::packComponent(Packer& packer, entt::entity handle, const Type& component,
                                      const SyncOperation op) {
    static constexpr auto id = EntityComponentIds::value<Type>;
    packer.pack_array(4);
    packer.pack(id);
    packer.pack(op);
    packer.pack(static_cast<uint32_t>(handle));
    packer.pack(component);
}

template <typename Type>
void ControllerNetwork::sendComponents(NetworkStream& peer, const ComponentReferences<Type>& components,
                                       const size_t count, const SyncOperation op) {

    NetworkStream::Writer writer{peer, PacketType::DataReliable};
    writer.start<MessageSceneUpdateEvent>(0);
    writer.pack_array(count);
    for (size_t i = 0; i < count; i++) {
        packComponent(writer, std::get<0>(components.at(i)), *std::get<1>(components.at(i)), op);
    }
    writer.flush();
}

template <typename View>
void ControllerNetwork::packComponents(NetworkStream& peer, const View& view, const SyncOperation op) {
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
void ControllerNetwork::unpackComponent(const uint64_t remoteId, const entt::entity handle, const msgpack::object& obj,
                                        const SyncOperation op) {
    if (op == SyncOperation::Emplace) {
        auto& component = reg.emplace<T>(handle);
        component.postUnpack(handle);
        obj.convert(component);
        postEmplaceComponent(remoteId, handle, component);
    } else if (op == SyncOperation::Patch) {
        auto* component = reg.try_get<T>(handle);
        if (component) {
            obj.convert(*component);
            postPatchComponent(remoteId, handle, *component);
        }
    } else {
        logger.warn("Unknown sync operation for entity id: {}", static_cast<uint32_t>(handle));
    }
}

void ControllerNetwork::unpackComponentId(const uint32_t id, const uint64_t remoteId, const entt::entity handle,
                                          const msgpack::object& obj, const SyncOperation op) {
    static std::unordered_map<uint32_t, UnpackerFunction> unpackers = {
        {EntityComponentIds::value<ComponentTransform>, &ControllerNetwork::unpackComponent<ComponentTransform>},
        {EntityComponentIds::value<ComponentRigidBody>, &ControllerNetwork::unpackComponent<ComponentRigidBody>},
        {EntityComponentIds::value<ComponentModel>, &ControllerNetwork::unpackComponent<ComponentModel>},
        {EntityComponentIds::value<ComponentModelSkinned>, &ControllerNetwork::unpackComponent<ComponentModelSkinned>},
        {EntityComponentIds::value<ComponentIcon>, &ControllerNetwork::unpackComponent<ComponentIcon>},
        {EntityComponentIds::value<ComponentLabel>, &ControllerNetwork::unpackComponent<ComponentLabel>},
        {EntityComponentIds::value<ComponentGrid>, &ControllerNetwork::unpackComponent<ComponentGrid>},
        {EntityComponentIds::value<ComponentTurret>, &ControllerNetwork::unpackComponent<ComponentTurret>},
        {EntityComponentIds::value<ComponentShipControl>, &ControllerNetwork::unpackComponent<ComponentShipControl>},
    };

    const auto found = unpackers.find(id);
    if (found != unpackers.end()) {
        (this->*(found->second))(remoteId, handle, obj, op);
    } else {
        logger.warn("Unmatched component id: {} entity id: {}", id, static_cast<uint32_t>(handle));
    }
}

void ControllerNetwork::sendFullSnapshot(NetworkStream& peer) {
    packComponents(peer, reg.view<ComponentTransform>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentRigidBody>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentModel>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentModelSkinned>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentIcon>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentLabel>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentGrid>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentTurret>(), SyncOperation::Emplace);
    packComponents(peer, reg.view<ComponentShipControl>(), SyncOperation::Emplace);
}

void ControllerNetwork::sendUpdate(NetworkStream& peer) {
    size_t count{0};
    size_t arraySize{0};
    auto total{updatedComponentsCount};

    NetworkStream::Writer writer{peer, PacketType::DataReliable};

    const auto prepareNext = [&]() {
        if (count >= arraySize) {
            writer.flush();

            arraySize = std::min<size_t>(total, 64);
            count = 0;
            writer.start<MessageSceneUpdateEvent>(0);
            writer.pack_array(arraySize);
        }
        ++count;
        --total;
    };

    for (const auto& pair : updatedComponentsMap) {
        const auto handle = pair.first;
        if (pair.second & componentMaskId<ComponentTransform>()) {
            const auto& component = reg.get<ComponentTransform>(handle);
            prepareNext();
            packComponent(writer, handle, component, SyncOperation::Patch);
        }
        if (pair.second & componentMaskId<ComponentRigidBody>()) {
            const auto& component = reg.get<ComponentRigidBody>(handle);
            prepareNext();
            packComponent(writer, handle, component, SyncOperation::Patch);
        }
        if (pair.second & componentMaskId<ComponentTurret>()) {
            const auto& component = reg.get<ComponentTurret>(handle);
            prepareNext();
            packComponent(writer, handle, component, SyncOperation::Patch);
        }
        if (pair.second & componentMaskId<ComponentShipControl>()) {
            const auto& component = reg.get<ComponentShipControl>(handle);
            prepareNext();
            packComponent(writer, handle, component, SyncOperation::Patch);
        }
    }

    writer.flush();

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
        const auto handle = child.ptr[2].as<EntityId>();

        // Map the remote entity ID to a local one
        auto local = remoteToLocal.find(handle);
        if (local == remoteToLocal.end() && op == SyncOperation::Emplace) {
            local = remoteToLocal.emplace(handle, reg.create()).first;
            localToRemote.emplace(local->second, handle);
        }

        if (local != remoteToLocal.end()) {
            unpackComponentId(id, static_cast<uint64_t>(local->first), local->second, child.ptr[3], op);
        } else {
            logger.warn("Unmatched entity update id: {}", static_cast<uint32_t>(handle));
        }
    }
}

void ControllerNetwork::onDestroyEntity(entt::registry& r, entt::entity handle) {
    (void)r;

    const auto it = updatedComponentsMap.find(handle);
    if (it != updatedComponentsMap.end()) {
        updatedComponentsCount -= std::bitset<64>{it->second}.count();
        updatedComponentsMap.erase(it);
    }
}

std::optional<Entity> ControllerNetwork::getRemoteToLocalEntity(const EntityId entity) const {
    auto local = remoteToLocal.find(entity);
    if (local != remoteToLocal.end()) {
        return Entity{reg, local->second};
    }
    return std::nullopt;
}

EntityId ControllerNetwork::getRemoteToLocal(const EntityId entity) const {
    auto local = remoteToLocal.find(entity);
    if (local != remoteToLocal.end()) {
        return local->second;
    }
    return NullEntity;
}

EntityId ControllerNetwork::getLocalToRemote(const EntityId entity) const {
    auto remote = localToRemote.find(entity);
    if (remote != localToRemote.end()) {
        return remote->second;
    }
    return NullEntity;
}
