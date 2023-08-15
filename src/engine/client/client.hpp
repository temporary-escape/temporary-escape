#pragma once

#include "../assets/assets_manager.hpp"
#include "../config.hpp"
#include "../future.hpp"
#include "../library.hpp"
#include "../network/network_tcp_client.hpp"
#include "../scene/scene.hpp"
#include "../server/sector.hpp"
#include "../utils/return_type.hpp"
#include "../utils/worker.hpp"
#include "../utils/yaml.hpp"
#include "local_cache.hpp"
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

    bool isReady() const {
        return getScene() != nullptr && flagCacheSync.load();
    }

    LocalCache& getCache() {
        return cache;
    }

    const LocalCache& getCache() const {
        return cache;
    }

    bool isConnected() const {
        return networkClient && networkClient->isConnected();
    }

    void handle(Request<MessagePlayerLocationEvent> req);
    void handle(Request<MessagePingRequest> req);
    void handle(Request<MessageModManifestsResponse> req);
    void handle(Request<MessageLoginResponse> req);
    void handle(Request<MessageFetchGalaxyResponse> req);
    void handle(Request<MessageFetchSystemsResponse> req);
    void handle(Request<MessageFetchFactionsResponse> req);
    void handle(Request<MessageFetchRegionsResponse> req);
    // void handleSceneSnapshot(const PeerPtr& peer, Network::RawMessage message);

    // Used by unit tests for synchronized assertions
    template <typename Fn> bool check(Fn&& fn) {
        auto promise = std::make_shared<Promise<bool>>();
        sync.post([promise, f = std::forward<Fn>(fn)]() { promise->resolve(f()); });
        return promise->future().get(std::chrono::seconds{1});
    }

private:
    void startCacheSync();
    void createScene(const SectorData& sector);

    const Config& config;
    AssetsManager& assetsManager;
    const PlayerLocalProfile& localProfile;
    VoxelShapeCache* voxelShapeCache{nullptr};
    LocalCache cache{};
    std::atomic_bool flagCacheSync{false};

    // Promise<void> loggedIn;
    BackgroundWorker worker;
    SynchronizedWorker sync;
    std::unique_ptr<NetworkTcpClient> networkClient;

    std::unique_ptr<Scene> scene;

    Promise<void> promiseLogin;
};
} // namespace Engine
