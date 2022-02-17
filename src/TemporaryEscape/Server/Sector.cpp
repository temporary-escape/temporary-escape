#include "Sector.hpp"
#include "../Utils/StringUtils.hpp"
#include "Server.hpp"

#define CMP "Sector"

using namespace Engine;

Sector::Sector(Server& server, Database& db, std::string compoundId)
    : server(server), db(db), compoundId(std::move(compoundId)), loaded(false) {

    Log::i(CMP, "Started sector: '{}'", this->compoundId);
}

Sector::~Sector() {
    Log::i(CMP, "Stopped sector: '{}'", compoundId);
}

void Sector::load() {
    if (loaded) {
        return;
    }

    loaded = true;
    const auto tokens = split(compoundId, "/");
    const auto& galaxyId = tokens.at(0);
    const auto& systemId = tokens.at(1);
    const auto& sectorId = tokens.at(2);

    try {
        server.getSectorLoader().populate(galaxyId, systemId, sectorId, scene);
    } catch (...) {
        EXCEPTION_NESTED("Failed to populate sector");
    }
}

void Sector::update(const float delta) {
    for (auto& entity : scene.getEntities()) {
        if (entity->isDirty()) {
            entity->clearDirty();
            entityDeltas.push_back(entity->getDelta());
        }
    }

    for (auto& player : players) {
        MessageEntitySync msg{};
        for (auto& entity : player.entitiesToSync) {
            msg.entities.push_back(entity);
            if (msg.entities.size() >= 32) {
                break;
            }
        }

        player.entitiesToSync.erase(player.entitiesToSync.begin(), player.entitiesToSync.begin() + msg.entities.size());

        if (!msg.entities.empty()) {
            player.ptr->send(msg);
        }
    }

    sync.post([=]() {
        try {
            scene.update(delta);
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Sector tick error");
        }
    });

    while (!entityDeltas.empty()) {
        MessageEntityDeltas msg{};
        for (const auto& delta : entityDeltas) {
            msg.deltas.push_back(delta);
            if (msg.deltas.size() >= 32) {
                break;
            }
        }

        entityDeltas.erase(entityDeltas.begin(), entityDeltas.begin() + msg.deltas.size());

        if (!msg.deltas.empty()) {
            for (auto& player : players) {
                player.ptr->send(msg);
            }
        }
    }
    entityDeltas.clear();

    sync.reset();
    sync.run();
}

void Sector::addPlayer(PlayerPtr player) {
    sync.post([this, player = std::move(player)]() {
        Log::i(CMP, "Adding player: '{}' to sector: '{}'", player->getId(), compoundId);
        players.emplace_back();
        players.back().ptr = player;

        const auto op = [](const EntityPtr& entity) {
            // EntityView view;
            // view.ptr = entity;
            return entity;
        };

        std::transform(scene.getEntities().begin(), scene.getEntities().end(),
                       std::back_inserter(players.back().entitiesToSync), op);

        auto location = server.getServices().players.getLocation(player->getId());
        MessageSectorChanged msg{location};
        player->send(msg);
    });
}

void Sector::eventEntityAdded(const EntityPtr& entity) {
    for (auto& player : players) {
        player.entitiesToSync.emplace_back();
        player.entitiesToSync.back() = entity;
    }
}
