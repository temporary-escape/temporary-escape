#include "world.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

#define HANDLE_REQUEST(Req, Res)                                                                                       \
    server.addHandler([this](const PeerPtr& peer, Req req) -> Res {                                                    \
        Res res{};                                                                                                     \
        this->handle(peer, std::move(req), res);                                                                       \
        return res;                                                                                                    \
    })
#define HANDLE_REQUEST_VOID(Req)                                                                                       \
    server.addHandler([this](const PeerPtr& peer, Req req) -> void { this->handle(peer, std::move(req)); })

World::World(const Config& config, AssetsManager& assetsManager, Database& db, PlayerSessions& playerSessions) :
    db{db}, playerSessions{playerSessions} {
}

void World::registerHandlers(Network::Server& server) {
    HANDLE_REQUEST(MessageFetchFactionRequest, MessageFetchFactionResponse);
    HANDLE_REQUEST(MessageFetchFactionsRequest, MessageFetchFactionsResponse);
    HANDLE_REQUEST(MessageFetchGalaxyRequest, MessageFetchGalaxyResponse);
    HANDLE_REQUEST(MessageFetchRegionRequest, MessageFetchRegionResponse);
    HANDLE_REQUEST(MessageFetchRegionsRequest, MessageFetchRegionsResponse);
    HANDLE_REQUEST(MessageFetchSectorRequest, MessageFetchSectorResponse);
    HANDLE_REQUEST(MessageFetchSectorsRequest, MessageFetchSectorsResponse);
    HANDLE_REQUEST(MessageFetchSystemRequest, MessageFetchSystemResponse);
    HANDLE_REQUEST(MessageFetchSystemsRequest, MessageFetchSystemsResponse);
    HANDLE_REQUEST(MessageFetchPlanetsRequest, MessageFetchPlanetsResponse);
}

void World::handle(const PeerPtr& peer, MessageFetchFactionRequest req, MessageFetchFactionResponse& res) {
    (void)playerSessions.getSession(peer);

    const auto found = db.find<FactionData>(fmt::format("{}", req.factionId));
    if (!found) {
        EXCEPTION("No such faction: '{}'", req.factionId);
    }

    res.faction = *found;
}

void World::handle(const PeerPtr& peer, MessageFetchFactionsRequest req, MessageFetchFactionsResponse& res) {
    (void)req;
    (void)playerSessions.getSession(peer);

    res.items = db.next<FactionData>("", req.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
}

void World::handle(const PeerPtr& peer, MessageFetchGalaxyRequest req, MessageFetchGalaxyResponse& res) {
    const auto session = playerSessions.getSession(peer);

    auto galaxy = db.find<GalaxyData>(req.galaxyId);
    if (!galaxy) {
        EXCEPTION("No such galaxy: '{}'", req.galaxyId);
    }
    res.name = galaxy->name;
}

void World::handle(const PeerPtr& peer, MessageFetchRegionRequest req, MessageFetchRegionResponse& res) {
    (void)playerSessions.getSession(peer);

    const auto found = db.find<RegionData>(fmt::format("{}/{}", req.galaxyId, req.regionId));
    if (!found) {
        EXCEPTION("No such region: {}/{}", req.galaxyId, req.regionId);
    }

    res.region = *found;
}

void World::handle(const PeerPtr& peer, MessageFetchRegionsRequest req, MessageFetchRegionsResponse& res) {
    (void)playerSessions.getSession(peer);

    res.items = db.next<RegionData>(fmt::format("{}/", req.galaxyId), req.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
}

void World::handle(const PeerPtr& peer, MessageFetchSectorRequest req, MessageFetchSectorResponse& res) {
    (void)playerSessions.getSession(peer);

    const auto found = db.find<SectorData>(fmt::format("{}/{}/{}", req.galaxyId, req.systemId, req.sectorId));
    if (!found) {
        EXCEPTION("No such sector: {}/{}/{}", req.galaxyId, req.systemId, req.sectorId);
    }

    res.sector = *found;
}

void World::handle(const PeerPtr& peer, MessageFetchSectorsRequest req, MessageFetchSectorsResponse& res) {
    (void)playerSessions.getSession(peer);

    res.items = db.next<SectorData>(fmt::format("{}/{}/", req.galaxyId, req.systemId), req.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
}

void World::handle(const PeerPtr& peer, MessageFetchSystemRequest req, MessageFetchSystemResponse& res) {
    (void)playerSessions.getSession(peer);

    const auto found = db.find<SystemData>(fmt::format("{}/{}", req.galaxyId, req.systemId));
    if (!found) {
        EXCEPTION("No such system: {}/{}", req.galaxyId, req.systemId);
    }

    res.system = *found;
}

void World::handle(const PeerPtr& peer, MessageFetchSystemsRequest req, MessageFetchSystemsResponse& res) {
    (void)playerSessions.getSession(peer);

    res.items = db.next<SystemData>(fmt::format("{}/", req.galaxyId), req.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
}

void World::handle(const PeerPtr& peer, MessageFetchPlanetsRequest req, MessageFetchPlanetsResponse& res) {
    (void)playerSessions.getSession(peer);

    res.items = db.next<PlanetData>(fmt::format("{}/{}/", req.galaxyId, req.systemId), req.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
}
