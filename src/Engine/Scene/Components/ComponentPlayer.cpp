#include "ComponentPlayer.hpp"

using namespace Engine;

ComponentPlayer::ComponentPlayer(EntityId entity, std::string playerId) :
    Component{entity}, playerId{std::move(playerId)} {
}
