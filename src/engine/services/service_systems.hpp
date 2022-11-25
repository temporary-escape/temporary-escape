#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(SystemData) {
    std::string id;
    std::string galaxyId;
    std::string regionId;
    std::string name;
    std::optional<std::string> factionId;
    Vector2 pos;
    uint64_t seed = 0;
    std::vector<std::string> connections;

    SCHEMA_DEFINE(id, galaxyId, regionId, name, factionId, pos, seed, connections);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("SystemData");
};

SCHEMA(SectorPlanetData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    bool isMoon{false};
    std::optional<std::string> planet{std::nullopt};
    Vector2 pos;
    // AssetPlanetPtr asset;

    SCHEMA_DEFINE(id, name, isMoon, planet, pos);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("SectorPlanetData");
};

struct MessageFetchSystemRequest {
    std::string galaxyId;
    std::string systemId;

    MESSAGE_DEFINE(MessageFetchSystemRequest, galaxyId, systemId);
};

struct MessageFetchSystemResponse {
    SystemData system;

    MESSAGE_DEFINE(MessageFetchSystemResponse, system);
};

struct MessageFetchSystemsRequest {
    std::string galaxyId;
    std::string token;

    MESSAGE_DEFINE(MessageFetchSystemsRequest, galaxyId, token);
};

struct MessageFetchSystemsResponse {
    std::vector<SystemData> systems;
    std::string token;
    bool hasNext{false};

    MESSAGE_DEFINE(MessageFetchSystemsResponse, systems, token, hasNext);
};

class ServiceSystems : public Service {
public:
    explicit ServiceSystems(const Config& config, Registry& registry, TransactionalDatabase& db, MsgNet::Server& server,
                            Service::SessionValidator& sessionValidator);

    void create(const SystemData& system);
    void create(const SectorPlanetData& planet);

    void handle(const PeerPtr& peer, MessageFetchSystemRequest req, MessageFetchSystemResponse& res);
    void handle(const PeerPtr& peer, MessageFetchSystemsRequest req, MessageFetchSystemsResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
