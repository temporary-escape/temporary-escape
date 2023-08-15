#include "service_factions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServiceFactions::ServiceFactions(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST(MessageFetchFactionRequest);
    HANDLE_REQUEST(MessageFetchFactionsRequest);
}

ServiceFactions::~ServiceFactions() = default;

void ServiceFactions::handle(Request<MessageFetchFactionRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchFactionRequest faction: {}", data.factionId);

    const auto found = db.find<FactionData>(fmt::format("{}", data.factionId));
    if (!found) {
        EXCEPTION("No such faction: '{}'", data.factionId);
    }

    MessageFetchFactionResponse res{};
    res.faction = *found;
    req.respond(res);
}

void ServiceFactions::handle(Request<MessageFetchFactionsRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchFactionsRequest token: {}", data.token);

    MessageFetchFactionsResponse res{};
    res.items = db.next<FactionData>("", data.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
    req.respond(res);
}
