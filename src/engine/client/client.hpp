#pragma once

#include "../assets/assets_manager.hpp"
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
    explicit Client(const Config& config, AssetsManager& assetsManager, const PlayerLocalProfile& localProfile);
    virtual ~Client();

    void connect(const std::string& address, int port);
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

    void handle(MessagePlayerLocationEvent res);
    void handle(MessagePingRequest req);
    void handleSceneSnapshot(const PeerPtr& peer, Network::RawMessage message);

private:
    void doLogin();
    void validateManifests(const std::vector<ModManifest>& serverManifests);
    void createScene(SectorData sector);
    /*void fetchModInfo(std::shared_ptr<Promise<void>> promise);
    void fetchLogin(std::shared_ptr<Promise<void>> promise);
    void fetchSpawnRequest(std::shared_ptr<Promise<void>> promise);
    void fetchSystemInfo();*/
    void onError(std::error_code ec) override;
    void onError(const PeerPtr& peer, std::error_code ec) override;
    void onUnhandledException(const PeerPtr& peer, std::exception_ptr& eptr) override;
    void postDispatch(std::function<void()> fn) override;

private:
    const Config& config;
    AssetsManager& assetsManager;
    const PlayerLocalProfile& localProfile;
    std::string playerId;
    PlayerLocationData playerLocation;

    // Promise<void> loggedIn;

    SynchronizedWorker sync;
    // PeriodicWorker worker1s{std::chrono::milliseconds(1000)};

    std::unique_ptr<Scene> scene;
};
} // namespace Engine
