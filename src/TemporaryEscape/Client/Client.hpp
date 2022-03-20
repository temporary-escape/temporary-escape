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
#include "PlayerLocalProfile.hpp"
#include "Request.hpp"
#include "Stats.hpp"
#include "Store.hpp"

namespace Engine {
class ENGINE_API Client : public NetworkTcpClient<ServerSink> {
public:
    explicit Client(const Config& config, Stats& stats, Store& store, const std::string& address, int port);
    virtual ~Client();

    void fetchGalaxy() {
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
    }

    void update();

    Scene* getScene() const {
        return scene.get();
    }

    template <typename T> void send(T& message) {
        NetworkTcpClient<ServerSink>::template send(message);
        ++stats.network.packetsSent;
    }

    void handle(MessageLogin::Response res) override;
    void handle(MessagePlayerLocation::Response res) override;
    void handle(MessageSceneEntities::Response res) override;
    void handle(MessageSceneDeltas::Response res) override;
    void handle(MessageFetchGalaxy::Response res) override;
    void handle(MessageFetchRegions::Response res) override;
    void handle(MessageFetchSystems::Response res) override;
    void handle(MessageShipMovement::Response res) override;

private:
    Stats& stats;
    Store& store;

    PlayerLocalProfile localProfile;
    std::string playerId;

    Promise<void> loggedIn;

    asio::io_service& sync;
    PeriodicWorker worker1s{std::chrono::milliseconds(1000)};

    std::unique_ptr<Scene> scene;
    std::shared_ptr<Entity> camera;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;
};
} // namespace Engine
