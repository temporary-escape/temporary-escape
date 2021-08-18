#include "Zone.hpp"

#include "../Assets/AssetManager.hpp"
#include "../Scene/ComponentModel.hpp"
#include "../Scene/ComponentParticleEmitter.hpp"
#include "Messages.hpp"
#include "World.hpp"

using namespace Scissio;

Zone::Zone(const Config& config, AssetManager& assetManager, World& world, Worker::Strand strand,
           const uint64_t sectorId)
    : config(config), assetManager(assetManager), world(world), strand(std::move(strand)), sectorId(sectorId),
      ready(false), warmup(0) {
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

    warmup = 1;
}

void Zone::tick() {
    strand.post([this]() { tickInternal(); });
}

void Zone::tickInternal() {
    if (warmup > 0) {
        --warmup;
        ready = warmup == 0;

        if (ready) {
            Log::v("Zone id: {} warmup complete", sectorId);
        }
    }

    scene.update();
}

void Zone::addPlayer(const PlayerSessionPtr& session) {
    if (players.find(session->getPlayerId()) == players.end()) {
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
    });
}
