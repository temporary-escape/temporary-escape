#include "service_galaxies.hpp"

using namespace Engine;

ServiceGalaxies::ServiceGalaxies(const Config& config, Registry& registry, TransactionalDatabase& db,
                                 Network::Server& server, Service::SessionValidator& sessionValidator) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator} {

    HANDLE_REQUEST(MessageFetchGalaxyRequest, MessageFetchGalaxyResponse);
}

GalaxyData ServiceGalaxies::get(const std::string& id) {
    const auto found = db.get<GalaxyData>(id);
    if (!found) {
        EXCEPTION("Galaxy '{}' does not exist", id);
    }

    return *found;
}

void ServiceGalaxies::create(const GalaxyData& galaxy) {
    db.put(galaxy.id, galaxy);
}

void ServiceGalaxies::handle(const PeerPtr& peer, MessageFetchGalaxyRequest req, MessageFetchGalaxyResponse& res) {
    const auto session = sessionValidator.find(peer);

    auto galaxy = get(req.galaxyId);
    res.name = galaxy.name;
}
