#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
struct MessageFetchGalaxyRequest {
    bool dummy{false};

    MSGPACK_DEFINE_ARRAY(dummy);
};

MESSAGE_DEFINE(MessageFetchGalaxyRequest);

struct MessageFetchGalaxyResponse {
    std::string galaxyId;
    std::string name;

    MSGPACK_DEFINE_ARRAY(name, galaxyId);
};

MESSAGE_DEFINE(MessageFetchGalaxyResponse);

class ServiceGalaxy : public Service {
public:
    explicit ServiceGalaxy(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceGalaxy();

private:
    void handle(Request<MessageFetchGalaxyRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
