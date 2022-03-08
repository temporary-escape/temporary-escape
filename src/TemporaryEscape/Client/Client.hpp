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
    explicit Client(Config& config, const std::string& address, int port);
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

    const Stats& getStats() const {
        return stats;
    }

    Scene* getScene() const {
        return scene.get();
    }

    Stats& getStats() {
        return stats;
    }

    Store& getStore() {
        return store;
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
    /*class EventListener : public Network::EventListener {
    public:
        explicit EventListener(Client& client) : client(client) {
        }

        void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override {
            (void)stream;
            client.eventPacket(std::move(packet));
        }

        void eventConnect(const Network::StreamPtr& stream) override {
            (void)stream;
            client.eventConnect();
        }

        void eventDisconnect(const Network::StreamPtr& stream) override {
            (void)stream;
            client.eventDisconnect();
        }

    private:
        Client& client;
    };*/

    /*void handle(MessageServerHello res);
    void handle(MessageLoginResponse res);
    void handle(MessageStatusResponse res);
    void handle(MessageSectorChanged res);
    void handle(MessageEntitySync res);
    void handle(MessageEntityDeltas res);
    template <typename Message, typename T = typename Message::Response::ItemType>
    void handleFetch(MessageFetchResponse<T> res);*/

    static std::atomic<uint64_t> nextRequestId;

    Store store;

    // std::unique_ptr<EventListener> listener;
    // std::shared_ptr<Network::Client> network;
    // Network::MessageDispatcher<> dispatcher;
    PlayerLocalProfile localProfile;
    std::string playerId;

    Promise<void> loggedIn;

    asio::io_service& sync;
    PeriodicWorker worker1s{std::chrono::milliseconds(1000)};
    // BackgroundWorker background;

    std::atomic<uint64_t> requestsNextId;
    std::mutex requestsMutex;
    std::unordered_map<uint64_t, AbstractRequestPtr> requests;

    Stats stats;

    std::unique_ptr<Scene> scene;
    std::shared_ptr<Entity> camera;

    std::chrono::time_point<std::chrono::steady_clock> lastTimePoint;
};
} // namespace Engine
