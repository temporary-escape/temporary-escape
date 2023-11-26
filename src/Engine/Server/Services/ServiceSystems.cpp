#include "ServiceSystems.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServiceSystems::ServiceSystems(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST(MessageFetchSystemRequest);
    HANDLE_REQUEST(MessageFetchSystemsRequest);
}

ServiceSystems::~ServiceSystems() = default;

void ServiceSystems::handle(Request<MessageFetchSystemRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchSystemRequest galaxy: {} system: {}", data.galaxyId, data.systemId);

    const auto found = db.find<SystemData>(fmt::format("{}/{}", data.galaxyId, data.systemId));
    if (!found) {
        EXCEPTION("No such system: {}/{}", data.galaxyId, data.systemId);
    }

    MessageFetchSystemResponse res{};
    res.system = *found;
    req.respond(res);
}

void ServiceSystems::handle(Request<MessageFetchSystemsRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchSystemsRequest galaxy: {} token: {}", data.galaxyId, data.token);

    MessageFetchSystemsResponse res{};
    res.items = db.next<SystemData>(fmt::format("{}/", data.galaxyId), data.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
    req.respond(res);
}
