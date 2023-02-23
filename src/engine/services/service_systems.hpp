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

SCHEMA(PlanetaryBodyData) {
    std::string id;
    std::string galaxyId;
    std::string systemId;
    std::string name;
    bool isMoon{false};
    std::optional<std::string> parent;
    Vector2 pos;

    SCHEMA_DEFINE(id, name, isMoon, parent, pos);
    SCHEMA_INDEXES_NONE();
    SCHEMA_NAME("PlanetaryBodyData");
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

struct MessageFetchPlanetaryBodiesRequest {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MESSAGE_DEFINE(MessageFetchPlanetaryBodiesRequest, galaxyId, systemId, token);
};

struct MessageFetchPlanetaryBodiesResponse {
    std::vector<PlanetaryBodyData> bodies;
    std::string token;
    bool hasNext{false};

    MESSAGE_DEFINE(MessageFetchPlanetaryBodiesResponse, bodies, token, hasNext);
};

class ENGINE_API ServiceSystems : public Service {
public:
    explicit ServiceSystems(const Config& config, Registry& registry, TransactionalDatabase& db, Network::Server& server,
                            Service::SessionValidator& sessionValidator);

    void create(const SystemData& system);
    void update(const SystemData& system);
    void create(const PlanetaryBodyData& planetaryBody);
    std::vector<SystemData> getForGalaxy(const std::string& galaxyId);
    std::vector<PlanetaryBodyData> getPlanetaryBodies(const std::string& galaxyId, const std::string& systemId);

    void handle(const PeerPtr& peer, MessageFetchSystemRequest req, MessageFetchSystemResponse& res);
    void handle(const PeerPtr& peer, MessageFetchSystemsRequest req, MessageFetchSystemsResponse& res);
    void handle(const PeerPtr& peer, MessageFetchPlanetaryBodiesRequest req, MessageFetchPlanetaryBodiesResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
