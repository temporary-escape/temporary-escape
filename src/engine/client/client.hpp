#pragma once

#include "../assets/registry.hpp"
#include "../config.hpp"
#include "../future.hpp"
#include "../library.hpp"
#include "../network/client.hpp"
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

class ENGINE_API Client : public Network::Client {
public:
    explicit Client(const Config& config, Registry& registry, Scene::Pipelines& scenePipelines, Stats& stats,
                    const Path& profilePath);
    virtual ~Client();

    Future<void> connect(const std::string& address, int port);
    void update();
    void stop();

    Scene* getScene() const {
        return scene.get();
    }

    const std::string& getPlayerId() const {
        return playerId;
    }

    const PlayerLocationData& getPlayerLocation() const {
        return playerLocation;
    }

    void handle(MessagePlayerLocationChanged res);
    void handle(MessageSceneEntitiesChanged res);
    void handle(MessageSceneDeltasChanged res);
    void handle(MessagePingRequest req);

    template <typename Fn> void onSectorUpdated(Fn&& fn) {
        callbackSystemUpdated = fn;
    }

private:
    void fetchModInfo(std::shared_ptr<Promise<void>> promise);
    void fetchLogin(std::shared_ptr<Promise<void>> promise);
    void fetchSpawnRequest(std::shared_ptr<Promise<void>> promise);
    void fetchSystemInfo();
    void onError(std::error_code ec) override;
    void onError(const PeerPtr& peer, std::error_code ec) override;
    void onUnhandledException(const PeerPtr& peer, std::exception_ptr& eptr) override;
    void postDispatch(std::function<void()> fn) override;

private:
    Registry& registry;
    Scene::Pipelines& scenePipelines;
    Stats& stats;
    PlayerLocalProfile localProfile;
    std::string playerId;
    PlayerLocationData playerLocation;

    // Promise<void> loggedIn;

    SynchronizedWorker sync;
    // PeriodicWorker worker1s{std::chrono::milliseconds(1000)};

    std::unique_ptr<Scene> scene;
    std::shared_ptr<Entity> camera;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;

    std::function<void(SystemData&)> callbackSystemUpdated;
};
} // namespace Engine