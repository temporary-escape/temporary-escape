#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../messages.hpp"
#include "../service.hpp"

namespace Engine {
struct MessagePlayerLocationRequest {
    bool dummy{false};

    MSGPACK_DEFINE(dummy);
};

MESSAGE_DEFINE(MessagePlayerLocationRequest);

struct MessagePlayerLocationResponse {
    std::optional<PlayerLocationData> location;

    MSGPACK_DEFINE(location);
};

MESSAGE_DEFINE(MessagePlayerLocationResponse);

class ServicePlayers : public Service {
public:
    explicit ServicePlayers(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServicePlayers();

    std::optional<std::string> secretToId(const uint64_t secret);
    PlayerData login(uint64_t secret, const std::string& name);
    PlayerLocationData getSpawnLocation(const std::string& id);

private:
    void handle(Request<MessagePlayerLocationRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
