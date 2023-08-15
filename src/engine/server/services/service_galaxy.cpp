#include "service_galaxy.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServiceGalaxy::ServiceGalaxy(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST(MessageFetchGalaxyRequest);
}

ServiceGalaxy::~ServiceGalaxy() = default;

void ServiceGalaxy::handle(Request<MessageFetchGalaxyRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchGalaxyRequest");

    // Get the main galaxy ID
    const auto mainGalaxyId = db.get<MetaData>("main_galaxy_id");
    const auto galaxyId = std::get<std::string>(mainGalaxyId.value);

    auto galaxy = db.find<GalaxyData>(galaxyId);
    if (!galaxy) {
        EXCEPTION("No such galaxy: '{}'", galaxyId);
    }

    MessageFetchGalaxyResponse res{};
    res.name = galaxy->name;
    res.galaxyId = galaxy->id;
    req.respond(res);
}
