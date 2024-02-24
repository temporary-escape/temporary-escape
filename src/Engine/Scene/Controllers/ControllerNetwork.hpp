#pragma once

#include "../Controller.hpp"
#include "../Entity.hpp"

namespace Engine {
class ENGINE_API NetworkStream;

enum class SyncOperation {
    Patch,
    Emplace,
    Remove,
};

class ENGINE_API ControllerNetwork : public Controller {
public:
    static constexpr const char* messageComponentSnapshotName = "MessageComponentSnapshot";

    explicit ControllerNetwork(Scene& scene, entt::registry& reg);
    ~ControllerNetwork() override;
    NON_COPYABLE(ControllerNetwork);
    NON_MOVEABLE(ControllerNetwork);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    void sendFullSnapshot(NetworkStream& peer);
    void sendUpdate(NetworkStream& peer);
    void receiveUpdate(const msgpack::object& obj);
    void resetUpdates();
    std::optional<Entity> getRemoteToLocalEntity(uint64_t id) const;
    EntityId getRemoteToLocal(EntityId entity) const;
    EntityId getLocalToRemote(EntityId entity) const;

private:
    struct ChildParentValue {
        uint64_t parentId;
        entt::entity child;
    };

    using UnpackerFunction = void (ControllerNetwork::*)(uint64_t, entt::entity, const msgpack::object&,
                                                         const SyncOperation op);
    template <typename T> using ComponentReferences = std::array<std::tuple<entt::entity, const T*>, 64>;

    template <typename Type> void postEmplaceComponent(uint64_t remoteId, entt::entity handle, Type& component);
    template <typename Type> void postPatchComponent(uint64_t remoteId, entt::entity handle, Type& component);

    template <typename T>
    void unpackComponent(uint64_t remoteId, entt::entity handle, const msgpack::object& obj, SyncOperation op);
    void unpackComponentId(const uint32_t id, uint64_t remoteId, entt::entity handle, const msgpack::object& obj,
                           SyncOperation op);

    template <typename Packer, typename Type>
    void packComponent(Packer& packer, entt::entity handle, const Type& component, const SyncOperation op);
    template <typename Type>
    void sendComponents(NetworkStream& peer, const ComponentReferences<Type>& components, const size_t count,
                        const SyncOperation op);
    template <typename View> void packComponents(NetworkStream& peer, const View& view, const SyncOperation op);

    template <typename T> void registerComponent() {
        reg.on_update<T>().template connect<&ControllerNetwork::onUpdateComponent<T>>(this);
        reg.on_destroy<T>().template connect<&ControllerNetwork::onDestroyComponent<T>>(this);
    }
    template <typename T> void unregisterComponent() {
        reg.on_update<T>().template disconnect<&ControllerNetwork::onUpdateComponent<T>>(this);
        reg.on_destroy<T>().template disconnect<&ControllerNetwork::onDestroyComponent<T>>(this);
    }

    template <typename T> void onUpdateComponent(entt::registry& r, entt::entity handle) {
        auto& value = updatedComponentsMap[handle];
        if (!(value & componentMaskId<T>())) {
            updatedComponentsMap[handle] |= componentMaskId<T>();
            ++updatedComponentsCount;
        }
    }
    template <typename T> void onDestroyComponent(entt::registry& r, entt::entity handle) {
        auto it = updatedComponentsMap.find(handle);
        if (it != updatedComponentsMap.end()) {
            if (it->second & componentMaskId<T>()) {
                it->second &= ~componentMaskId<T>();
                --updatedComponentsCount;
            }
        }
    }
    void onDestroyEntity(entt::registry& r, entt::entity handle);

    Scene& scene;
    entt::registry& reg;
    std::unordered_map<EntityId, EntityId> remoteToLocal;
    std::unordered_map<EntityId, EntityId> localToRemote;
    std::unordered_map<entt::entity, uint64_t> updatedComponentsMap;
    size_t updatedComponentsCount{0};
    std::vector<ChildParentValue> transformChildParentMap;
};
} // namespace Engine

MSGPACK_ADD_ENUM(Engine::SyncOperation);
