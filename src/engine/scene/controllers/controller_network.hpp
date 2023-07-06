#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
namespace Network {
class ENGINE_API Peer;
}

enum class SyncOperation {
    Patch,
    Emplace,
    Remove,
};

class ENGINE_API ControllerNetwork : public Controller {
public:
    explicit ControllerNetwork(entt::registry& reg);
    ~ControllerNetwork() override;

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    void sendFullSnapshot(Network::Peer& peer);
    void sendUpdate(Network::Peer& peer);
    void receiveUpdate(const msgpack::object& obj);
    void resetUpdates();

private:
    void onUpdateTransform(entt::registry& r, entt::entity handle);

    entt::registry& reg;
    std::unordered_map<entt::entity, entt::entity> remoteToLocal;
    std::list<std::tuple<entt::entity, ComponentTransform*>> toSendTransforms;
};
} // namespace Engine

MSGPACK_ADD_ENUM(Engine::SyncOperation);
