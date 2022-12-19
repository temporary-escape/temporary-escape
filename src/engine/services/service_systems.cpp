#include "service_systems.hpp"

using namespace Engine;

#define CMP "ServiceSystems"

ServiceSystems::ServiceSystems(const Config& config, Registry& registry, TransactionalDatabase& db,
                               Network::Server& server, Service::SessionValidator& sessionValidator) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator} {

    HANDLE_REQUEST(MessageFetchSystemRequest, MessageFetchSystemResponse);
    HANDLE_REQUEST(MessageFetchSystemsRequest, MessageFetchSystemsResponse);
    HANDLE_REQUEST(MessageFetchPlanetaryBodiesRequest, MessageFetchPlanetaryBodiesResponse);
}

/*std::vector<SystemData> ServiceSystems::getForPlayer(const std::string& playerId, const std::string& galaxyId,
                                                     const std::string& start, std::string& next) {
    return db.next<SystemData>(fmt::format("{}/", galaxyId), start, 64, &next);
}

std::vector<SectorPlanetData> ServiceSystems::getPlanets(const std::string& galaxyId, const std::string& systemId) {
    return db.seekAll<SectorPlanetData>(fmt::format("{}/{}/", galaxyId, systemId));
}*/

void ServiceSystems::create(const SystemData& system) {
    db.put(fmt::format("{}/{}", system.galaxyId, system.id), system);
}

void ServiceSystems::update(const SystemData& system) {
    db.update<SystemData>(fmt::format("{}/{}", system.galaxyId, system.id), [&](auto& found) -> bool {
        found = system;
        return true;
    });
}

void ServiceSystems::create(const PlanetaryBodyData& planetaryBody) {
    db.put(fmt::format("{}/{}/{}", planetaryBody.galaxyId, planetaryBody.systemId, planetaryBody.id), planetaryBody);
}

std::vector<SystemData> ServiceSystems::getForGalaxy(const std::string& galaxyId) {
    return db.seekAll<SystemData>(fmt::format("{}/", galaxyId));
}

std::vector<PlanetaryBodyData> ServiceSystems::getPlanetaryBodies(const std::string& galaxyId,
                                                                  const std::string& systemId) {
    return db.seekAll<PlanetaryBodyData>(fmt::format("{}/{}/", galaxyId, systemId));
}

void ServiceSystems::handle(const PeerPtr& peer, MessageFetchSystemRequest req, MessageFetchSystemResponse& res) {
    (void)sessionValidator.find(peer);

    const auto found = db.get<SystemData>(fmt::format("{}/{}", req.galaxyId, req.systemId));
    if (!found) {
        EXCEPTION("No such system: {}/{}", req.galaxyId, req.systemId);
    }

    res.system = *found;
}

void ServiceSystems::handle(const PeerPtr& peer, MessageFetchSystemsRequest req, MessageFetchSystemsResponse& res) {
    (void)sessionValidator.find(peer);

    res.systems = db.next<SystemData>(fmt::format("{}/", req.galaxyId), req.token, 64, &res.token);
    res.hasNext = res.systems.size() == 64;
}

void ServiceSystems::handle(const Service::PeerPtr& peer, MessageFetchPlanetaryBodiesRequest req,
                            MessageFetchPlanetaryBodiesResponse& res) {
    (void)sessionValidator.find(peer);

    res.bodies =
        db.next<PlanetaryBodyData>(fmt::format("{}/{}/", req.galaxyId, req.systemId), req.token, 64, &res.token);
    res.hasNext = res.bodies.size() == 64;
}
