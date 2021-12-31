#include "Sector.hpp"
#include "Server.hpp"

#define CMP "Sector"

using namespace Scissio;

Sector::Sector(Server& server, Database& db, std::string compoundId)
    : server(server), db(db), compoundId(std::move(compoundId)) {

    Log::i(CMP, "Started sector {}", compoundId);
}

Sector::~Sector() {
    Log::i(CMP, "Stopped sector {}", compoundId);
}

/*void Sector::load(GeneratorChain& generator) {
    const auto found = db.get<SectorData>(compoundId);
    if (!found) {
        EXCEPTION("Failed to get sector data for {}", compoundId);
    }

    const auto& data = found.value();
    generator.populate(db, data.seed, scene, data.galaxyId, data.systemId, data.id);
}*/

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
    sync.post([=]() {
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

        MessageSectorChanged msg{compoundId};
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
