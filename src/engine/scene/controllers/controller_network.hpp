#pragma once

#include "../controller.hpp"
#include "../entity.hpp"

namespace Engine {
namespace Network {
class ENGINE_API Peer;
}

class ENGINE_API ControllerNetwork : public Controller {
public:
    explicit ControllerNetwork(entt::registry& reg);
    ~ControllerNetwork() override;

    void update(float delta) override;
    void recalculate(VulkanRenderer& vulkan) override;

    void sendSnapshot(Network::Peer& peer);
    void receiveSnapshot(const msgpack::object& obj);

private:
    entt::registry& reg;
    std::unordered_map<entt::entity, entt::entity> remoteToLocal;
};
} // namespace Engine
