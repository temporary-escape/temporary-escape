#pragma once

#include "../assets/registry.hpp"
#include "../config.hpp"
#include "../database/database.hpp"
#include "../future.hpp"
#include "../library.hpp"
#include "../utils/worker.hpp"
#include "generator.hpp"
#include "messages.hpp"
#include "sector.hpp"
#include "session.hpp"
#include <msgnet/server.hpp>
#include <shared_mutex>

namespace Engine {
class ENGINE_API Server : public MsgNet::Server, public Service::SessionValidator {
public:
    struct Certs {
        Certs() : key{}, cert{key}, dh{} {
        }

        MsgNet::Pkey key;
        MsgNet::Cert cert;
        MsgNet::Dh dh;
    };

    explicit Server(const Config& config, const Certs& certs, Registry& registry, TransactionalDatabase& db);
    virtual ~Server();

    Future<void> load();
    void stop();
    void tick();

    void onAcceptSuccess(PeerPtr peer) override;
    void handle(const PeerPtr& peer, MessageLoginRequest req, MessageLoginResponse& res);
    void handle(const PeerPtr& peer, MessageModsInfoRequest req, MessageModsInfoResponse& res);
    void handle(const PeerPtr& peer, MessageSpawnRequest req, MessageSpawnResponse& res);
    void handle(const PeerPtr& peer, MessageShipMovementRequest req, MessageShipMovementResponse& res);
    void handle(const PeerPtr& peer, MessagePingResponse res);

    std::shared_ptr<Service::Session> find(const std::shared_ptr<MsgNet::Peer>& peer) override;

private:
    void addPeerToLobby(const PeerPtr& peer);
    void removePeerFromLobby(const PeerPtr& peer);
    SessionPtr createSession(const PeerPtr& peer, const PlayerData& player);
    SessionPtr peerToSession(const PeerPtr& peer);
    std::tuple<SessionPtr, SectorPtr> peerToSessionSector(const PeerPtr& peer);
    bool isPeerLoggedIn(const std::string& playerId);
    void disconnectPeer(const PeerPtr& peer);
    SectorPtr startSector(const std::string& galaxyId, const std::string& systemId, const std::string& sectorId);
    void addPlayerToSector(const SessionPtr& session, const std::string& sectorId);
    std::vector<SessionPtr> getAllSessions();
    void updateSessionsPing(const std::vector<SessionPtr>& sessions);

public:
    void onError(std::error_code ec) override;
    void onError(const PeerPtr& peer, std::error_code ec) override;
    void onUnhandledException(const PeerPtr& peer, std::exception_ptr& eptr) override;
    void postDispatch(std::function<void()> fn) override;

private:
    const Config& config;
    Registry& registry;
    TransactionalDatabase& db;
    World world;
    Generator generator;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    BackgroundWorker worker;
    Worker::Strand commands;

    // BackgroundWorker& worker;
    // BackgroundWorker loader;

    struct {
        std::shared_mutex mutex;
        std::unordered_set<MsgNet::Peer*> lobby;
        std::unordered_map<MsgNet::Peer*, SessionPtr> sessions;
    } players;

    struct {
        std::shared_mutex mutex;
        std::unordered_map<std::string, std::shared_ptr<Sector>> map;
    } sectors;

    std::unique_ptr<MsgNet::Server> server;
};
} // namespace Engine
