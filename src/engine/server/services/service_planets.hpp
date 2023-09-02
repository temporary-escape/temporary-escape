#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
struct MessageFetchPlanetsRequest {
    std::string galaxyId;
    std::string systemId;
    std::string token;

    MSGPACK_DEFINE_ARRAY(galaxyId, systemId, token);
};

MESSAGE_DEFINE(MessageFetchPlanetsRequest);

struct MessageFetchPlanetsResponse : MessagePage<PlanetData> {
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(MessagePage<PlanetData>));
};

MESSAGE_DEFINE(MessageFetchPlanetsResponse);

class ServicePlanets : public Service {
public:
    explicit ServicePlanets(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServicePlanets();

private:
    void handle(Request<MessageFetchPlanetsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
