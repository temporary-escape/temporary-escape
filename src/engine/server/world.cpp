#include "world.hpp"

using namespace Engine;

static auto logger = createLogger(__FILENAME__);

World::World(const Config& config, Registry& registry, TransactionalDatabase& db, Network::Server& server,
             Service::SessionValidator& sessionValidator) :
    players{config, registry, db, server, sessionValidator},
    galaxies{config, registry, db, server, sessionValidator},
    regions{config, registry, db, server, sessionValidator},
    factions{config, registry, db, server, sessionValidator},
    systems{config, registry, db, server, sessionValidator},
    sectors{config, registry, db, server, sessionValidator},
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
