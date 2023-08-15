#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
struct MessageFetchFactionRequest {
    std::string factionId;

    MSGPACK_DEFINE(factionId);
};

MESSAGE_DEFINE(MessageFetchFactionRequest);

struct MessageFetchFactionResponse {
    FactionData faction;

    MSGPACK_DEFINE(faction);
};

MESSAGE_DEFINE(MessageFetchFactionResponse);

struct MessageFetchFactionsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchFactionsRequest);

struct MessageFetchFactionsResponse : MessagePage<FactionData> {
    MSGPACK_DEFINE(MSGPACK_BASE(MessagePage<FactionData>));
};

MESSAGE_DEFINE(MessageFetchFactionsResponse);

class ServiceFactions : public Service {
public:
    explicit ServiceFactions(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceFactions();

private:
    void handle(Request<MessageFetchFactionRequest> req);
    void handle(Request<MessageFetchFactionsRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
