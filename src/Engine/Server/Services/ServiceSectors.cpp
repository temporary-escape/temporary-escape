#include "ServiceSectors.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServiceSectors::ServiceSectors(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST(MessageFetchSectorRequest);
    HANDLE_REQUEST(MessageFetchSectorsRequest);
}

ServiceSectors::~ServiceSectors() = default;

void ServiceSectors::handle(Request<MessageFetchSectorRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchSectorRequest galaxy: {} system: {} sector: {}",
                 data.galaxyId,
                 data.systemId,
                 data.sectorId);

    const auto found = db.find<SectorData>(fmt::format("{}/{}/{}", data.galaxyId, data.systemId, data.sectorId));
    if (!found) {
        EXCEPTION("No such sector: {}/{}/{}", data.galaxyId, data.systemId, data.sectorId);
    }

    MessageFetchSectorResponse res{};
    res.sector = *found;
    req.respond(res);
}

void ServiceSectors::handle(Request<MessageFetchSectorsRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug(
        "Handle MessageFetchSectorsRequest galaxy: {} system: {} token: {}", data.galaxyId, data.systemId, data.token);

    MessageFetchSectorsResponse res{};
    res.items =
        db.next<SectorData>(fmt::format("{}/{}/", data.galaxyId, data.systemId), data.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
    req.respond(res);
}
