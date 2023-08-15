#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
class ENGINE_API NetworkPeer;

enum class SyncOperation {
    Patch,
    Emplace,
    Remove,
};

class ENGINE_API ControllerNetwork : public Controller {
public:
    static constexpr const char* messageComponentSnapshotName = "MessageComponentSnapshot";

    explicit ControllerNetwork(entt::registry& reg);
    ~ControllerNetwork() override;
    NON_COPYABLE(ControllerNetwork);
    NON_MOVEABLE(ControllerNetwork);

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    void sendFullSnapshot(NetworkPeer& peer);
    void sendUpdate(NetworkPeer& peer);
    void receiveUpdate(const msgpack::object& obj);
    void resetUpdates();

private:
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

    entt::registry& reg;
    std::unordered_map<entt::entity, entt::entity> remoteToLocal;
    std::unordered_map<entt::entity, uint64_t> updatedComponentsMap;
    size_t updatedComponentsCount{0};
};
} // namespace Engine

MSGPACK_ADD_ENUM(Engine::SyncOperation);
