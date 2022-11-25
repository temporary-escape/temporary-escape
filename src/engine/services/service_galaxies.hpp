#pragma once

#include "service.hpp"

namespace Engine {
SCHEMA(GalaxyData) {
    std::string id;
    std::string name;
    Vector2 pos;
    uint64_t seed = 0;

    SCHEMA_DEFINE(id, name, pos, seed);
    SCHEMA_INDEXES(name);
    SCHEMA_NAME("GalaxyData");
};

struct MessageFetchGalaxyRequest {
    std::string galaxyId;

    MESSAGE_DEFINE(MessageFetchGalaxyRequest, galaxyId);
};

struct MessageFetchGalaxyResponse {
    std::string name;

    MESSAGE_DEFINE(MessageFetchGalaxyResponse, name);
};

class ServiceGalaxies : public Service {
public:
    explicit ServiceGalaxies(const Config& config, Registry& registry, TransactionalDatabase& db,
                             MsgNet::Server& server, Service::SessionValidator& sessionValidator);

    void create(const GalaxyData& galaxy);

    void handle(const PeerPtr& peer, MessageFetchGalaxyRequest req, MessageFetchGalaxyResponse& res);

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    Service::SessionValidator& sessionValidator;
};
} // namespace Engine
