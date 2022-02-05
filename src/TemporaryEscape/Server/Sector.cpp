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

void Sector::update() {
    for (auto& player : players) {
        MessageEntitySync msg{};
        for (auto& view : player.entities) {
            if (view.sync) {
                msg.entities.push_back(view.ptr);
                view.sync = false;
            }

            if (msg.entities.size() >= 32) {
                break;
            }
        }

        if (!msg.entities.empty()) {
            player.ptr->send(msg);
        }
    }

    sync.post([=]() {
        try {
            scene.update();
        } catch (std::exception& e) {
            BACKTRACE(CMP, e, "Sector tick error");
        }
    });

    sync.reset();
    sync.run();
}

void Sector::addPlayer(PlayerPtr player) {
    sync.post([this, player = std::move(player)]() {
        Log::i(CMP, "Adding player: '{}' to sector: '{}'", player->getId(), compoundId);
        players.emplace_back();
        players.back().ptr = player;

        const auto op = [](const EntityPtr& entity) {
            EntityView view;
            view.ptr = entity;
            return view;
        };

        std::transform(scene.getEntities().begin(), scene.getEntities().end(),
                       std::back_inserter(players.back().entities), op);

        auto location = server.getServices().players.getLocation(player->getId());
        MessageSectorChanged msg{location};
        player->send(msg);
    });
}

void Sector::eventEntityAdded(const EntityPtr& entity) {
    for (auto& player : players) {
        player.entities.emplace_back();
        auto& view = player.entities.back();
        view.ptr = entity;
    }
}
