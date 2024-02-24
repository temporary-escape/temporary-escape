#pragma once

#include "../../Database/Database.hpp"
#include "../../Network/NetworkDispatcher.hpp"
#include "../Messages.hpp"
#include "../Service.hpp"

namespace Engine {
struct MessagePlayerLocationRequest {
    bool dummy{false};

    MSGPACK_DEFINE_ARRAY(dummy);
};

MESSAGE_DEFINE(MessagePlayerLocationRequest);

struct MessagePlayerLocationResponse {
    std::optional<PlayerLocationData> location;

    MSGPACK_DEFINE_ARRAY(location);
};

MESSAGE_DEFINE(MessagePlayerLocationResponse);

class ServicePlayers : public Service {
public:
    explicit ServicePlayers(NetworkDispatcher2& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServicePlayers();

    std::optional<std::string> secretToId(const uint64_t secret);
    PlayerData login(uint64_t secret, const std::string& name);
    PlayerLocationData getSpawnLocation(const std::string& id);

private:
    void handle(Request2<MessagePlayerLocationRequest> req);

    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
