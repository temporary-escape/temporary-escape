#pragma once

/*#include "../services/service_factions.hpp"
#include "../services/service_galaxies.hpp"
#include "../services/service_players.hpp"
#include "../services/service_regions.hpp"
#include "../services/service_sectors.hpp"
#include "../services/service_systems.hpp"*/

#include "../assets/assets_manager.hpp"
#include "../database/database.hpp"
#include "../network/server.hpp"
#include "../utils/event_bus.hpp"
#include "player_sessions.hpp"
#include "schemas.hpp"

namespace Engine {
class ENGINE_API World {
public:
    explicit World(const Config& config, AssetsManager& assetsManager, Database& db, PlayerSessions& playerSessions);
    ~World() = default;

    void registerHandlers(Network::Server& server);

private:
    // Faction requests
    void handle(const PeerPtr& peer, MessageFetchFactionRequest req, MessageFetchFactionResponse& res);
    void handle(const PeerPtr& peer, MessageFetchFactionsRequest req, MessageFetchFactionsResponse& res);

    // Galaxy requests
    void handle(const PeerPtr& peer, MessageFetchGalaxyRequest req, MessageFetchGalaxyResponse& res);

    // Region requests
    void handle(const PeerPtr& peer, MessageFetchRegionRequest req, MessageFetchRegionResponse& res);
    void handle(const PeerPtr& peer, MessageFetchRegionsRequest req, MessageFetchRegionsResponse& res);

    // Sector requests
    void handle(const PeerPtr& peer, MessageFetchSectorRequest req, MessageFetchSectorResponse& res);
    void handle(const PeerPtr& peer, MessageFetchSectorsRequest req, MessageFetchSectorsResponse& res);

    // System requests
    void handle(const PeerPtr& peer, MessageFetchSystemRequest req, MessageFetchSystemResponse& res);
    void handle(const PeerPtr& peer, MessageFetchSystemsRequest req, MessageFetchSystemsResponse& res);

    // Planet requests
    void handle(const PeerPtr& peer, MessageFetchPlanetsRequest req, MessageFetchPlanetsResponse& res);

    Database& db;
    PlayerSessions& playerSessions;
};
} // namespace Engine
