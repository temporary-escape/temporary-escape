#include "ServiceRegions.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServiceRegions::ServiceRegions(NetworkDispatcher2& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {

    HANDLE_REQUEST2(MessageFetchRegionRequest);
    HANDLE_REQUEST2(MessageFetchRegionsRequest);
}

ServiceRegions::~ServiceRegions() = default;

void ServiceRegions::handle(Request2<MessageFetchRegionRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchRegionRequest galaxy: {} region: {}", data.galaxyId, data.regionId);

    MessageFetchRegionResponse res{};
    const auto found = db.find<RegionData>(fmt::format("{}/{}", data.galaxyId, data.regionId));
    if (!found) {
        EXCEPTION("No such region: {}/{}", data.galaxyId, data.regionId);
    }

    res.region = *found;
    req.respond(res);
}

void ServiceRegions::handle(Request2<MessageFetchRegionsRequest> req) {
    (void)sessions.getSession(req.peer);
    const auto data = req.get();

    logger.debug("Handle MessageFetchRegionsRequest galaxy: {} token: {}", data.galaxyId, data.token);

    MessageFetchRegionsResponse res{};
    res.items = db.next<RegionData>(fmt::format("{}/", data.galaxyId), data.token, 64, &res.page.token);
    res.page.hasNext = res.items.size() == 64;
    req.respond(res);
}
