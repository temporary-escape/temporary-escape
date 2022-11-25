#include "world.hpp"

#define CMP "World"

using namespace Engine;

World::World(const Config& config, Registry& registry, Engine::TransactionalDatabase& db, MsgNet::Server& server,
             Service::SessionValidator& sessionValidator) :
    players{config, registry, db, server, sessionValidator},
    galaxies{config, registry, db, server, sessionValidator},
    regions{config, registry, db, server, sessionValidator},
    factions{config, registry, db, server, sessionValidator},
    systems{config, registry, db, server, sessionValidator},
    sectors{config, registry, db, server, sessionValidator} {
}
