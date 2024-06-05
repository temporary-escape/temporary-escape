#pragma once

#include "MatchmakerClient.hpp"

namespace Engine {
class ENGINE_API MatchmakerSession : public BackgroundWorker, public NetworkWebsockets::Receiver {
public:
    struct EventStatus {
        inline static const std::string_view type = "Status";

        int numPlayers;
    };

    struct EventConnectionRequest {
        inline static const std::string_view type = "ConnectionRequest";

        std::string id;
        std::string address;
        int port;
    };

    struct EventConnectionResponse {
        inline static const std::string_view type = "ConnectionResponse";

        std::string id;
        std::string address;
        int port;
    };

    class Receiver {
    public:
        virtual ~Receiver() = default;

        virtual void onStunRequest(EventConnectionRequest event) = 0;
    };

    MatchmakerSession(MatchmakerClient& client, Receiver& receiver, std::string serverName);
    ~MatchmakerSession();

    void close();
    void sendStunResponse(const std::string& id, const asio::ip::udp::endpoint& endpoint);

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
    Receiver& receiver;
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

inline void to_json(Json& j, const MatchmakerSession::EventConnectionResponse& m) {
    j = Json{
        {"id", m.id},
        {"address", m.address},
        {"port", m.port},
    };
}

inline void from_json(const Json& j, MatchmakerSession::EventConnectionRequest& m) {
    j.at("id").get_to(m.id);
    j.at("address").get_to(m.address);
    j.at("port").get_to(m.port);
}
} // namespace Engine
