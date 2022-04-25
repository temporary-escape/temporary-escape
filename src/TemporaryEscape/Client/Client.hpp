#pragma once

#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkTcpClient.hpp"
#include "../Scene/Scene.hpp"
#include "../Server/Messages.hpp"
#include "../Server/Schemas.hpp"
#include "../Server/Sector.hpp"
#include "../Utils/Worker.hpp"
#include "../Utils/Yaml.hpp"
#include "Request.hpp"
#include "Stats.hpp"

namespace Engine {
struct PlayerLocalProfile {
    std::string name;
    uint64_t secret;

    YAML_DEFINE(name, secret);
};

class ENGINE_API Client : public NetworkTcpClient<Client, ServerSink> {
public:
    explicit Client(const Config& config, Stats& stats, const std::string& address, int port);
    virtual ~Client();

    /*void fetchGalaxy() {
        MessageFetchGalaxy::Request req{};
        req.galaxyId = store.player.location.value().galaxyId;
        send(req);
    }

    void fetchGalaxySystems() {
        MessageFetchSystems::Request req{};
        req.galaxyId = store.player.location.value().galaxyId;
        send(req);
    }

    void fetchGalaxyRegions() {
        MessageFetchRegions::Request req{};
        req.galaxyId = store.player.location.value().galaxyId;
        send(req);
    }*/

    void update();

    Scene* getScene() const {
        return scene.get();
    }

    const std::string& getPlayerId() const {
        return playerId;
    }

    const PlayerLocationData& getPlayerLocation() const {
        return playerLocation;
    }

    template <typename M, typename Fn> void send(M& message, Fn&& callback) {
        NetworkTcpClient<Client, ServerSink>::template send(message, std::forward<Fn>(callback));
        ++stats.network.packetsSent;
    }

    // void handle(MessageLogin::Response res);
    void handle(MessagePlayerLocation::Response res);
    void handle(MessageSceneEntities::Response res);
    void handle(MessageSceneDeltas::Response res);
    // void handle(MessageFetchGalaxy::Response res);
    // void handle(MessageFetchRegions::Response res);
    // void handle(MessageFetchSystems::Response res);
    // void handle(MessageShipMovement::Response res);

private:
    Stats& stats;

    PlayerLocalProfile localProfile;
    std::string playerId;
    PlayerLocationData playerLocation;

    // Promise<void> loggedIn;

    asio::io_service& sync;
    PeriodicWorker worker1s{std::chrono::milliseconds(1000)};

    std::unique_ptr<Scene> scene;
    std::shared_ptr<Entity> camera;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;
};
} // namespace Engine
