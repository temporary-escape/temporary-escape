#pragma once

#include "../../Database/Database.hpp"
#include "../../Network/NetworkDispatcher.hpp"
#include "../Messages.hpp"
#include "../Schemas.hpp"
#include "../Service.hpp"

namespace Engine {
struct MessageFetchRegionRequest {
    std::string galaxyId;
    std::string regionId;

    MSGPACK_DEFINE_ARRAY(galaxyId, regionId);
};

MESSAGE_DEFINE(MessageFetchRegionRequest);

struct MessageFetchRegionResponse {
    RegionData region;

    MSGPACK_DEFINE_ARRAY(region);
};

MESSAGE_DEFINE(MessageFetchRegionResponse);

struct MessageFetchRegionsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE_ARRAY(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchRegionsRequest);

struct MessageFetchRegionsResponse : MessagePage<RegionData> {
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(MessagePage<RegionData>));
};

MESSAGE_DEFINE(MessageFetchRegionsResponse);

class ServiceRegions : public Service {
public:
    explicit ServiceRegions(NetworkDispatcher2& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceRegions();

private:
    void handle(Request2<MessageFetchRegionRequest> req);
    void handle(Request2<MessageFetchRegionsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
