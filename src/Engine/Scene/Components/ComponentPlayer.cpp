#include "ComponentPlayer.hpp"

using namespace Engine;

ComponentPlayer::ComponentPlayer(entt::registry& reg, entt::entity handle, std::string playerId) :
    Component{reg, handle}, playerId{std::move(playerId)} {
}
