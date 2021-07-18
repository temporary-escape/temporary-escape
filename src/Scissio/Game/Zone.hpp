#pragma once

#include "../Config.hpp"
#include "../Scene/Scene.hpp"
#include "../Utils/Worker.hpp"
#include "PlayerSession.hpp"

namespace Scissio {
class AssetManager;
class Server;
class World;

class SCISSIO_API Zone {
public:
    explicit Zone(const Config& config, AssetManager& assetManager, World& world, Worker::Strand strand,
                  uint64_t sectorId);
    virtual ~Zone();

    void load();
    void tick();

    uint64_t getSectorId() const {
        return sectorId;
    }

    bool isReady() const {
        return ready;
    }

    void addPlayer(const PlayerSessionPtr& session);

private:
    void tickInternal();
    template <typename T> void sendAll(const size_t channel, const T& message) {
        for (const auto& [playerId, session] : players) {
            session->send(channel, message);
        }
    }

    const Config& config;
    AssetManager& assetManager;
    World& world;
    Worker::Strand strand;
    const uint64_t sectorId;
    bool ready;
    int warmup;

    Scene scene;
    std::unordered_map<uint64_t, PlayerSessionPtr> players;
};

using ZonePtr = std::shared_ptr<Zone>;
} // namespace Scissio
