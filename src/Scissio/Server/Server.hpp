#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Future.hpp"
#include "../Library.hpp"
#include "../Network/NetworkServer.hpp"
#include "../Utils/Worker.hpp"
#include "Database.hpp"
#include "Messages.hpp"
#include "Player.hpp"
#include "Schemas.hpp"
#include "Sector.hpp"
#include "Services/ServiceGalaxies.hpp"
#include "Services/ServicePlayers.hpp"
#include "Services/ServiceRegions.hpp"
#include "Services/ServiceSectors.hpp"
#include "Services/ServiceSystems.hpp"

namespace Scissio {
class SCISSIO_API Server {
public:
    struct Services {
        explicit Services(const Config& config, AssetManager& assetManager, Database& db);

        ServiceGalaxies galaxies;
        ServiceRegions regions;
        ServiceSystems systems;
        ServiceSectors sectors;
        ServicePlayers players;
    };

    explicit Server(const Config& config, AssetManager& assetManager, Database& db);
    virtual ~Server();

    // void tick();

    void eventConnect(const Network::StreamPtr& stream);
    void eventDisconnect(const Network::StreamPtr& stream);
    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet);

    std::vector<PlayerPtr> getPlayers();

private:
    class EventListener : public Network::EventListener {
    public:
        explicit EventListener(Server& server) : server(server) {
        }

        void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override {
            server.eventPacket(stream, std::move(packet));
        }

        void eventConnect(const Network::StreamPtr& stream) override {
            server.eventConnect(stream);
        }

        void eventDisconnect(const Network::StreamPtr& stream) override {
            server.eventDisconnect(stream);
        }

    private:
        Server& server;
    };

    struct SectorReference {
        std::string compoundId;
        std::shared_ptr<Sector> ptr;
        bool ready;
    };

    void handle(const Network::StreamPtr& stream, MessageLoginRequest req);
    void handle(const Network::StreamPtr& stream, MessageStatusRequest req);
    template <typename RequestType> void handleFetch(const Network::StreamPtr& stream, RequestType req);

    template <typename RequestType>
    typename RequestType::Response::ItemType fetch(const PlayerPtr& player, const RequestType& req, std::string& next);

    PlayerPtr streamToPlayer(const Network::StreamPtr& stream, bool check = true);

    const Config& config;
    AssetManager& assetManager;
    Database& db;
    Services services;
    Promise<void> loaded;

    Network::MessageDispatcher<const Network::StreamPtr&> dispatcher;

    // std::thread tickThread;
    // std::atomic_bool tickFlag;

    // Worker worker;
    // Worker::Strand strand;
    BackgroundWorker worker;
    BackgroundWorker loader;

    std::unique_ptr<EventListener> listener;
    std::shared_ptr<Network::Server> network;

    struct PlayersInternal {
        std::shared_mutex mutex;
        std::unordered_map<Network::StreamPtr, PlayerPtr> map;
    } players;

    std::vector<SectorReference> sectors;
};
} // namespace Scissio
