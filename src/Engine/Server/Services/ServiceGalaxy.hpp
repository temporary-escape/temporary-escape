#pragma once

#include "../../Database/Database.hpp"
#include "../../Network/NetworkDispatcher.hpp"
#include "../Messages.hpp"
#include "../Schemas.hpp"
#include "../Service.hpp"

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
    explicit ServiceGalaxy(NetworkDispatcher2& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceGalaxy();

private:
    void handle(Request2<MessageFetchGalaxyRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
