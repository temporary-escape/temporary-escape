#pragma once

#include "../Assets/AssetsManager.hpp"
#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkUdpClient.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Sector.hpp"
#include "../Utils/ReturnType.hpp"
#include "../Utils/Worker.hpp"
#include "LocalCache.hpp"
#include "Stats.hpp"

namespace Engine {
struct PlayerLocalProfile {
    std::string name;
    uint64_t secret;

    void convert(const Xml::Node& xml) {
        xml.convert("name", name);
        xml.convert("secret", secret);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("name", name);
        xml.pack("secret", secret);
    }
};

XML_DEFINE(PlayerLocalProfile, "profile");

class ENGINE_API Client : public NetworkDispatcher2 {
public:
    explicit Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile,
                    VoxelShapeCache* voxelShapeCache, const std::string& address, int port);
    virtual ~Client();

    void update(float deltaTime);
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
        return network && network->isEstablished();
    }

    void handle(Request2<MessagePlayerLocationEvent> req);
    void handle(Request2<MessagePingRequest> req);
    void handle(Request2<MessageModManifestsResponse> req);
    void handle(Request2<MessageLoginResponse> req);
    void handle(Request2<MessageFetchGalaxyResponse> req);
    void handle(Request2<MessageFetchSystemsResponse> req);
    void handle(Request2<MessageFetchFactionsResponse> req);
    void handle(Request2<MessageFetchRegionsResponse> req);
    void handle(Request2<MessageSceneUpdateEvent> req);
    // void handle(Request<MessageSceneBulletsEvent> req);
    void handle(Request2<MessagePlayerControlEvent> req);

    // Used by unit tests for synchronized assertions
    template <typename Fn> bool check(Fn&& fn) {
        auto promise = std::make_shared<Promise<bool>>();
        sync.post([promise, f = std::forward<Fn>(fn)]() { promise->resolve(f()); });
        return promise->future().get(std::chrono::seconds{1});
    }

    template <typename T> void send(const T& msg) {
        if (network) {
            network->send(msg);
        }
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
    std::shared_ptr<NetworkUdpClient> network;

    std::unique_ptr<Scene> scene;

    Promise<void> promiseLogin;
};
} // namespace Engine
