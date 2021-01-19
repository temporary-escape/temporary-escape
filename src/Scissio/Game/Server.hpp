#pragma once

#include "../Assets/AssetManager.hpp"
#include "../Config.hpp"
#include "../Network/NetworkServer.hpp"
#include "Messages.hpp"

namespace Scissio {
class SCISSIO_API Server : public Network::Server {
public:
    explicit Server(const Config& config, AssetManager& assetManager);
    virtual ~Server();

    void dispatch(const Network::SessionPtr& session, const Network::Packet& packet) override;
    Network::SessionPtr createSession(int64_t playerId, const std::string& password) override;
    void acceptSession(const Network::SessionPtr& session) override;

private:
    class Player : public Network::Session, std::enable_shared_from_this<Player> {
    public:
        explicit Player(const uint64_t sessionId, const int64_t playerId)
            : Network::Session(sessionId), playerId(playerId) {
        }
        virtual ~Player() = default;

        int64_t getPlaterId() const {
            return playerId;
        }

    private:
        int64_t playerId;
    };

    using PlayerPtr = std::shared_ptr<Player>;

    void handle(const PlayerPtr& player, MessageClientHello message);

    const Config& config;
    AssetManager& assetManager;

    std::shared_mutex playerMutex;
    std::list<PlayerPtr> players;
    std::list<PlayerPtr> playersLobby;
};
} // namespace Scissio
