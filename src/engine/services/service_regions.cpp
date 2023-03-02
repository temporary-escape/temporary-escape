#include "service_regions.hpp"

using namespace Engine;

ServiceRegions::ServiceRegions(const Config& config, Registry& registry, TransactionalDatabase& db,
                               Network::Server& server, Service::SessionValidator& sessionValidator,
                               EventBus& eventBus) :
    config{config}, registry{registry}, db{db}, sessionValidator{sessionValidator}, eventBus{eventBus} {

    HANDLE_REQUEST(MessageFetchRegionRequest, MessageFetchRegionResponse)
    HANDLE_REQUEST(MessageFetchRegionsRequest, MessageFetchRegionsResponse)
}

void ServiceRegions::create(const RegionData& region) {
    db.put(fmt::format("{}/{}", region.galaxyId, region.id), region);
}

std::vector<RegionData> ServiceRegions::getForGalaxy(const std::string& galaxyId) {
    return db.seekAll<RegionData>(fmt::format("{}/", galaxyId));
}

void ServiceRegions::handle(const PeerPtr& peer, MessageFetchRegionRequest req, MessageFetchRegionResponse& res) {
    (void)sessionValidator.find(peer);

    const auto found = db.get<RegionData>(fmt::format("{}/{}", req.galaxyId, req.regionId));
    if (!found) {
        EXCEPTION("No such region: {}/{}", req.galaxyId, req.regionId);
    }

    res.region = *found;
}

void ServiceRegions::handle(const PeerPtr& peer, MessageFetchRegionsRequest req, MessageFetchRegionsResponse& res) {
    (void)sessionValidator.find(peer);

    res.regions = db.next<RegionData>(fmt::format("{}/", req.galaxyId), req.token, 64, &res.token);
    res.hasNext = res.regions.size() == 64;
}
