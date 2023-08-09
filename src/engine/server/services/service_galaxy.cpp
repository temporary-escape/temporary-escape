#include "service_galaxy.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

ServiceGalaxy::ServiceGalaxy(NetworkDispatcher& dispatcher, Database& db, PlayerSessions& sessions) :
    db{db}, sessions{sessions} {
}

ServiceGalaxy::~ServiceGalaxy() = default;
