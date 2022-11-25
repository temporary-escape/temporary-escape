#include "service_galaxies.hpp"

using namespace Engine;

ServiceGalaxies::ServiceGalaxies(const Config& config, Registry& registry, TransactionalDatabase& db,
                                 MsgNet::Server& server, Service::SessionValidator& sessionValidator) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator} {

    HANDLE_REQUEST(MessageFetchGalaxyRequest, MessageFetchGalaxyResponse);
}

void ServiceGalaxies::create(const GalaxyData& galaxy) {
    db.put(galaxy.id, galaxy);
}

void ServiceGalaxies::handle(const PeerPtr& peer, MessageFetchGalaxyRequest req, MessageFetchGalaxyResponse& res) {
    const auto session = sessionValidator.find(peer);

    auto galaxy = db.get<GalaxyData>(req.galaxyId);
    if (!galaxy) {
        EXCEPTION("Galaxy '{}' does not exist", req.galaxyId);
    }

    res.name = galaxy->name;
}
