#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
struct MessageFetchRegionRequest {
    std::string galaxyId;
    std::string regionId;

    MSGPACK_DEFINE(galaxyId, regionId);
};

MESSAGE_DEFINE(MessageFetchRegionRequest);

struct MessageFetchRegionResponse {
    RegionData region;

    MSGPACK_DEFINE(region);
};

MESSAGE_DEFINE(MessageFetchRegionResponse);

struct MessageFetchRegionsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchRegionsRequest);

struct MessageFetchRegionsResponse : MessagePage<RegionData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<RegionData>));
};

MESSAGE_DEFINE(MessageFetchRegionsResponse);

class ServiceRegions : public Service {
public:
    explicit ServiceRegions(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceRegions();

private:
    void handle(Request<MessageFetchRegionRequest> req);
    void handle(Request<MessageFetchRegionsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine