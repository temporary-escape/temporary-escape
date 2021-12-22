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

void Sector::load(Generator& generator) {
    const auto found = db.get<SectorData>(compoundId);
    if (!found) {
        EXCEPTION("Failed to get sector data for {}", compoundId);
    }

    const auto& data = found.value();
    generator.populate(db, data.seed, scene, data.galaxyId, data.systemId, data.id);
}

void Sector::update() {
    for (auto& session : sessions) {
        MessageEntitySync msg{};
        for (auto& view : session.entities) {
            if (view.sync) {
                msg.entities.push_back(view.ptr);
                view.sync = false;
            }

            if (msg.entities.size() >= 32) {
                break;
            }
        }

        if (!msg.entities.empty()) {
            session.ptr->stream->send(msg);
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

void Sector::addSession(SessionPtr session) {
    sync.post([=]() {
        Log::i(CMP, "Adding player: '{}' to sector: '{}'", session->playerId, compoundId);
        sessions.emplace_back();
        sessions.back().ptr = session;

        const auto op = [](const EntityPtr& entity) {
            EntityView view;
            view.ptr = entity;
            return view;
        };

        std::transform(scene.getEntities().begin(), scene.getEntities().end(),
                       std::back_inserter(sessions.back().entities), op);

        MessageSectorChanged msg{compoundId};
        session->stream->send(msg);
    });
}

void Sector::eventEntityAdded(const EntityPtr& entity) {
    for (auto& session : sessions) {
        session.entities.emplace_back();
        auto& view = session.entities.back();
        view.ptr = entity;
    }
}
