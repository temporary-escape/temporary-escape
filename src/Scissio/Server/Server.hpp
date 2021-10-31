#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Network/NetworkTcpServer.hpp"
#include "../Utils/Worker.hpp"
#include "Database.hpp"
#include "Messages.hpp"
#include "Schemas.hpp"
#include "Session.hpp"

namespace Scissio {
class SCISSIO_API Server : public Network::TcpServer {
public:
    explicit Server(const Config& config, AssetManager& assetManager, Database& db);
    ~Server() override;

    void tick();

    void eventConnect(const Network::StreamPtr& stream) override;
    void eventDisconnect(const Network::StreamPtr& stream) override;
    void eventPacket(const Network::StreamPtr& stream, Network::Packet packet) override;

private:
    void handle(const Network::StreamPtr& stream, MessageLoginRequest req);
    SessionPtr getSession(const Network::StreamPtr& stream);

    const Config& config;
    AssetManager& assetManager;
    Database& db;

    Network::MessageDispatcher<const Network::StreamPtr&> dispatcher;

    std::thread tickThread;
    std::atomic_bool tickFlag;

    Worker worker;
    Worker::Strand strand;

    std::shared_mutex sessionsMutex;
    std::unordered_map<Network::StreamPtr, SessionPtr> sessions;
};
} // namespace Scissio
