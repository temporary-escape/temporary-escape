#pragma once

#include "MatchmakerClient.hpp"

namespace Engine {
class ENGINE_API MatchmakerSession : public BackgroundWorker, public NetworkWebsockets::Receiver {
public:
    struct EventStatus {
        inline static const std::string_view type = "Status";

        int numPlayers;
    };

    MatchmakerSession(MatchmakerClient& client, std::string serverName);
    ~MatchmakerSession();

    void close();

private:
    void doPing();
    void doRegister();
    void startWs();
    void startPing();
    void onWsReceive(Json json) override;
    void onWsConnect() override;
    void onWsConnectFailed() override;
    void onWsClose(int code) override;

    MatchmakerClient& client;
    std::string serverName;
    std::string url;
    std::atomic<bool> stopFlag{false};
    std::shared_ptr<NetworkWebsockets> ws;
    std::unique_ptr<asio::steady_timer> retry;
    std::unique_ptr<asio::steady_timer> reconnect;
    std::unique_ptr<asio::steady_timer> ping;
    std::string serverId;
};

inline void to_json(Json& j, const MatchmakerSession::EventStatus& m) {
    j = Json{
        {"num_players", m.numPlayers},
    };
}
} // namespace Engine
