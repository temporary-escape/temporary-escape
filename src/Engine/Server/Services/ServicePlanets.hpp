#pragma once

#include "../../Database/Database.hpp"
#include "../../Network/NetworkDispatcher.hpp"
#include "../Messages.hpp"
#include "../Schemas.hpp"
#include "../Service.hpp"

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
    explicit ServicePlanets(NetworkDispatcher2& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServicePlanets();

private:
    void handle(Request2<MessageFetchPlanetsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
