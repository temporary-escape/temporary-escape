#include "ServicePlanets.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServicePlanets::ServicePlanets(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST(MessageFetchPlanetsRequest);
}

ServicePlanets::~ServicePlanets() = default;

void ServicePlanets::handle(Request<MessageFetchPlanetsRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchPlanetsRequest galaxy: {} system: {}", data.galaxyId, data.systemId);

    MessageFetchPlanetsResponse res{};
    res.items =
        db.next<PlanetData>(fmt::format("{}/{}/", data.galaxyId, data.systemId), data.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
    req.respond(res);
}
