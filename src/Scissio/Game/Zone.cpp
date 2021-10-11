#include "Zone.hpp"

#include "../Assets/AssetManager.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/ComponentParticleEmitter.hpp"
#include "Messages.hpp"
#include "World.hpp"

using namespace Scissio;

#define STRAND(...)                                                                                                    \
    try {                                                                                                              \
        strand.post(__VA_ARGS__);                                                                                      \
    } catch (std::exception & e) {                                                                                     \
        BACKTRACE(e, "async work error");                                                                              \
    }

Zone::Zone(const Config& config, AssetManager& assetManager, World& world, Worker::Strand strand,
           const uint64_t sectorId)
    : config(config), assetManager(assetManager), world(world), strand(std::move(strand)), sectorId(sectorId),
      ready(false) {
}

Zone::~Zone() = default;

void Zone::load() {
    auto model = assetManager.find<Model>("model_asteroid_01_a");

    std::mt19937_64 rng;
    for (auto x = 0; x < 8; x++) {
        for (auto y = 0; y < 8; y++) {
            if (x == 0 && y == 0) {
                continue;
            }
            auto entity = scene.addEntity();
            entity->addComponent<ComponentModel>(model);
            entity->translate({x * 3.0f, 0.0f, y * 3.0f});
            entity->rotate(randomQuaternion(rng));
        }
    }

    auto texture = assetManager.find<BasicTexture>("star_dust");

    auto entity = scene.addEntity();
    entity->addComponent<ComponentParticleEmitter>(texture);

    ready = true;
}

void Zone::tick() {
    STRAND([this]() { scene.update(); });
}

void Zone::removePlayer(const PlayerSessionPtr& session) {
    STRAND([this, session]() {
        if (const auto it = players.find(session->getPlayerId()); it != players.end()) {
            Log::v("Removing player: {} from zone: {}", session->getPlayerId(), this->sectorId);
            players.erase(it);
        }
    });
}

void Zone::addPlayer(const PlayerSessionPtr& session) {
    STRAND([this, session]() {
        if (const auto it = players.find(session->getPlayerId()); it == players.end()) {
            Log::v("Adding player: {} to zone: {}", session->getPlayerId(), this->sectorId);
            players.insert(std::make_pair(session->getPlayerId(), session));
        }
    });
    /*if (players.find(session->getPlayerId()) == players.end()) {
        players.insert(std::make_pair(session->getPlayerId(), session));
    }

    strand.post([this]() {
        MessageEntityBatch batch{};
        for (const auto& entity : scene.getEntities()) {
            batch.entities.push_back(entity);
            if (batch.entities.size() >= 32) {
                sendAll(0, batch);
                batch.entities.clear();
            }
        }

        if (!batch.entities.empty()) {
            sendAll(0, batch);
        }
    });*/
}
