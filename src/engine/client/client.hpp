#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../future.hpp"
#include "../library.hpp"
#include "../network/network_tcp_client.hpp"
#include "../scene/scene.hpp"
#include "../server/messages.hpp"
#include "../server/sector.hpp"
#include "../utils/return_type.hpp"
#include "../utils/worker.hpp"
#include "../utils/yaml.hpp"
#include "stats.hpp"

namespace Engine {
struct PlayerLocalProfile {
    std::string name;
    uint64_t secret;

    YAML_DEFINE(name, secret);
};

class ENGINE_API Client : public NetworkDispatcher {
public:
    explicit Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile,
                    VoxelShapeCache* voxelShapeCache, const std::string& address, int port);
    virtual ~Client();

    void update();
    void disconnect();

    Scene* getScene() const {
        return scene.get();
    }

    const std::string& getPlayerId() const {
        return playerId;
    }

    const PlayerLocationData& getPlayerLocation() const {
        return playerLocation;
    }

    bool isConnected() const {
        return networkClient && networkClient->isConnected();
    }

    void handle(Request<MessagePlayerLocationEvent> req);
    void handle(Request<MessagePingRequest> req);
    // void handleSceneSnapshot(const PeerPtr& peer, Network::RawMessage message);

private:
    void doLogin();
    void validateManifests(const std::vector<ModManifest>& serverManifests);
    void createScene(const SectorData& sector);

private:
    const Config& config;
    AssetsManager& assetsManager;
    const PlayerLocalProfile& localProfile;
    VoxelShapeCache* voxelShapeCache{nullptr};
    std::string playerId;
    PlayerLocationData playerLocation;

    // Promise<void> loggedIn;
    BackgroundWorker worker;
    SynchronizedWorker sync;
    std::unique_ptr<NetworkTcpClient> networkClient;
    // PeriodicWorker worker1s{std::chrono::milliseconds(1000)};

    std::unique_ptr<Scene> scene;

    bool hasNetworkError{false};
};
} // namespace Engine
