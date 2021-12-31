#include "ServicePlayers.hpp"

using namespace Scissio;

ServicePlayers::ServicePlayers(const Config& config, AssetManager& assetManager, Scissio::Database& db)
    : config(config), assetManager(assetManager), db(db) {
}

void ServicePlayers::tick() {
}

PlayerLocationData ServicePlayers::getLocation(const std::string& playerId) {
    auto location = db.get<PlayerLocationData>(playerId);
    if (!location) {
        EXCEPTION("Player {} has no location", playerId);
    }
    return location.value();
}
