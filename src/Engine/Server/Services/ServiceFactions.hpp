#pragma once

#include "../../Database/Database.hpp"
#include "../../Network/NetworkDispatcher.hpp"
#include "../Messages.hpp"
#include "../Schemas.hpp"
#include "../Service.hpp"

namespace Engine {
struct MessageFetchFactionRequest {
    std::string factionId;

    MSGPACK_DEFINE_ARRAY(factionId);
};

MESSAGE_DEFINE(MessageFetchFactionRequest);

struct MessageFetchFactionResponse {
    FactionData faction;

    MSGPACK_DEFINE_ARRAY(faction);
};

MESSAGE_DEFINE(MessageFetchFactionResponse);

struct MessageFetchFactionsRequest {
    std::string galaxyId;
    std::string token;

    MSGPACK_DEFINE_ARRAY(galaxyId, token);
};

MESSAGE_DEFINE(MessageFetchFactionsRequest);

struct MessageFetchFactionsResponse : MessagePage<FactionData> {
    MSGPACK_DEFINE_ARRAY(MSGPACK_BASE_ARRAY(MessagePage<FactionData>));
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
