#pragma once

#include "../../database/database.hpp"
#include "../../network/network_dispatcher.hpp"
#include "../schemas.hpp"
#include "../service.hpp"

namespace Engine {
class ServiceGalaxy : public Service {
public:
    explicit ServiceGalaxy(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions);
    virtual ~ServiceGalaxy();

private:
    Database& db;
    PlayerSessions& sessions;
};
} // namespace Engine
