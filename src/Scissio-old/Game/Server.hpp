#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Network/NetworkServer.hpp"
#include "Messages.hpp"
#include "PlayerSession.hpp"
#include "World.hpp"
#include "Zone.hpp"

namespace Scissio {
class SCISSIO_API Server : public Network::Server {
public:
    explicit Server(const Config& config, AssetManager& assetManager, Database& db);
    virtual ~Server();

    void load();
    void eventConnect(const Network::StreamPtr& stream) override;
    void eventDisconnect(const Network::StreamPtr& stream) override;
    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override;

private:
    void tick();
    void dispatch(PlayerSessionPtr session, Network::Packet packet);
    void handle(PlayerSessionPtr session, MessageHelloRequest req);
    void handle(PlayerSessionPtr session, MessageSystemsRequest req);
    void handle(PlayerSessionPtr session, MessageRegionsRequest req);
    void handle(PlayerSessionPtr session, MessageBlocksRequest req);
    void handle(PlayerSessionPtr session, MessageSectorStatusRequest req);

    // uint64_t createSessionId() const;
    // void preparePlayer(const PlayerSessionPtr& session, const Player& player);
    // void prepareZone(const Sector& sector);
    // std::optional<ZonePtr> findZoneById(uint64_t id);
    // std::optional<ZonePtr> findZoneByPlayer(uint64_t id);
    // std::optional<ZonePtr> findZoneByPlayer(const PlayerSessionPtr& session);
    void login(const Network::StreamPtr& stream, MessageLoginRequest req);
    std::optional<PlayerSessionPtr> findPlayerByStream(const Network::StreamPtr& stream);
    ZonePtr prepareZone(const Sector& sector);

    const Config& config;
    AssetManager& assetManager;
    Database& db;

    Network::MessageDispatcher<PlayerSessionPtr> dispatcher;

    World world;
    Worker worker;
    Worker::Strand strand;
    Service loader;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    // std::shared_mutex glock;
    // std::vector<PlayerSessionPtr> players;
    // std::vector<PlayerSessionPtr> playersLobby;
    std::shared_mutex playersMutex;
    std::vector<Network::StreamPtr> lobby;
    std::unordered_map<Network::StreamPtr, PlayerSessionPtr> streamToPlayer;
    std::unordered_map<uint64_t, PlayerSessionPtr> players;

    std::unordered_map<uint64_t, ZonePtr> zones;
    std::unordered_map<uint64_t, ZonePtr> playerToZone;
};
} // namespace Scissio