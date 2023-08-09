#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../service.hpp"

namespace Engine {
class ServicePlayers : public Service {
public:
    explicit ServicePlayers(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServicePlayers();

    std::optional<std::string> secretToId(const uint64_t secret);
    PlayerData login(uint64_t secret, const std::string& name);

private:
    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
