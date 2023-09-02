#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
struct MessageFetchSystemRequest {
    std::string galaxyId;
    std::string systemId;

    MSGPACK_DEFINE_ARRAY(galaxyId, systemId);
};

MESSAGE_DEFINE(MessageFetchSystemRequest);

struct MessageFetchSystemResponse {
    SystemData system;

    MSGPACK_DEFINE_ARRAY(system);
};

MESSAGE_DEFINE(MessageFetchSystemResponse);

struct MessageFetchSystemsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE_ARRAY(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchSystemsRequest);

struct MessageFetchSystemsResponse : MessagePage<SystemData> {
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(MessagePage<SystemData>));
};

MESSAGE_DEFINE(MessageFetchSystemsResponse);

class ServiceSystems : public Service {
public:
    explicit ServiceSystems(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceSystems();

private:
    void handle(Request<MessageFetchSystemRequest> req);
    void handle(Request<MessageFetchSystemsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
