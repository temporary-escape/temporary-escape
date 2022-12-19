#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(RegionData) {
    std::string id;
    std::string galaxyId;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    SCHEMA_DEFINE(id, galaxyId, name, pos, seed);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("RegionData");
};

struct MessageFetchRegionRequest {
    std::string galaxyId;
    std::string regionId;

    MESSAGE_DEFINE(MessageFetchRegionRequest, galaxyId, regionId);
};

struct MessageFetchRegionResponse {
    RegionData region;

    MESSAGE_DEFINE(MessageFetchRegionResponse, region);
};

struct MessageFetchRegionsRequest {
    std::string galaxyId;
    std::string token;

    MESSAGE_DEFINE(MessageFetchRegionsRequest, galaxyId, token);
};

struct MessageFetchRegionsResponse {
    std::vector<RegionData> regions;
    std::string token;
    bool hasNext{false};

    MESSAGE_DEFINE(MessageFetchRegionsResponse, regions, token, hasNext);
};

class ServiceRegions : public Service {
public:
    explicit ServiceRegions(const Config& config, Registry& registry, TransactionalDatabase& db, Network::Server& server,
                            Service::SessionValidator& sessionValidator);

    void create(const RegionData& region);
    std::vector<RegionData> getForGalaxy(const std::string& galaxyId);
    void handle(const PeerPtr& peer, MessageFetchRegionRequest req, MessageFetchRegionResponse& res);
    void handle(const PeerPtr& peer, MessageFetchRegionsRequest req, MessageFetchRegionsResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
