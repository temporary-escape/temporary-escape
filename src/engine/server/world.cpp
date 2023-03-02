#include "world.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

World::World(const Config& config, Registry& registry, TransactionalDatabase& db, Network::Server& server,
             Service::SessionValidator& sessionValidator, EventBus& eventBus) :
    players{config, registry, db, server, sessionValidator, eventBus},
    galaxies{config, registry, db, server, sessionValidator, eventBus},
    regions{config, registry, db, server, sessionValidator, eventBus},
    factions{config, registry, db, server, sessionValidator, eventBus},
    systems{config, registry, db, server, sessionValidator, eventBus},
    sectors{config, registry, db, server, sessionValidator, eventBus},
    db{db} {
}

IndexData World::getIndex() {
    auto data = db.get<IndexData>("");
    if (!data) {
        data = IndexData{};
        db.put("", *data);
    }
    return *data;
}

IndexData World::updateIndex(const std::function<void(IndexData&)>& callback) {
    auto [_, updated] = db.update<IndexData>("", [&](std::optional<IndexData>& data) {
        if (!data) {
            data = IndexData{};
        }
        callback(*data);
        return true;
    });

    if (!updated) {
        EXCEPTION("Something went wrong while updating the world index");
    }

    return *updated;
}
