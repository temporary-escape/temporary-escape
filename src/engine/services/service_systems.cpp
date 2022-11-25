#include "service_systems.hpp"

using namespace Engine;

#define CMP "ServiceSystems"

ServiceSystems::ServiceSystems(const Config& config, Registry& registry, TransactionalDatabase& db,
                               MsgNet::Server& server, Service::SessionValidator& sessionValidator) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator} {

    HANDLE_REQUEST(MessageFetchSystemRequest, MessageFetchSystemResponse);
    HANDLE_REQUEST(MessageFetchSystemsRequest, MessageFetchSystemsResponse);
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

void ServiceSystems::create(const SectorPlanetData& planet) {
    db.put(fmt::format("{}/{}/{}", planet.galaxyId, planet.systemId, planet.id), planet);
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
