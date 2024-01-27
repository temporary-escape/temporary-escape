#pragma once

#include "../Network/NetworkWebsockets.hpp"
#include "../Utils/Worker.hpp"
#include <unordered_map>

namespace nlohmann {
template <> struct adl_serializer<asio::ip::udp::endpoint> {
    static void to_json(json& j, const asio::ip::udp::endpoint& e) {
        j = json{
            {"address", e.address().to_string()},
            {"port", e.port()},
        };
    }

    static void from_json(const json& j, asio::ip::udp::endpoint& e) {
        e = asio::ip::udp::endpoint{
            asio::ip::address::from_string(j.at("address").get<std::string>()),
            j.at("port").get<uint16_t>(),
        };
    }
};
} // namespace nlohmann

namespace Engine {
class ENGINE_API Matchmaker : public BackgroundWorker, public NetworkWebsockets::Receiver {
public:
    class Listener {
    public:
        virtual void onMatchmakerConnect() = 0;
        virtual void onMatchmakerDisconnect() = 0;
    };

    struct MessageHello {
        static constexpr auto type = "hello";

        std::string description;
    };

    struct MessageServerRegister {
        static constexpr auto type = "server_register";

        std::string name;
        asio::ip::udp::endpoint endpoint;
    };

    struct MessageServerRegistered {
        static constexpr auto type = "server_registered";

        std::string id;
    };

    struct MessageServerPing {
        static constexpr auto type = "server_ping";

        std::chrono::system_clock::time_point time;
    };

    Matchmaker(Listener& listener, std::string url);
    ~Matchmaker();

    void serverRegister(std::string name, const asio::ip::udp::endpoint& endpoint);
    void close();

private:
    using HandlerFunc = std::function<void(Json)>;

    template <typename T> void send(const T& value);
    void doPing();
    void retryWsConnection();
    void onReceive(Json json) override;
    void onConnect() override;
    void onConnectFailed() override;
    void onClose() override;

    void handle(const MessageHello& msg);
    void handle(const MessageServerRegistered& msg);

    template <typename T> void addHandler() {
        handlers.emplace(T::type, [this](Json json) { handle(json.template get<T>()); });
    }

    Listener& listener;
    std::string url;
    std::shared_ptr<NetworkWebsockets> ws;
    std::unordered_map<std::string, HandlerFunc> handlers;
    // Promise<void> promise;
    std::unique_ptr<asio::steady_timer> retry;
    std::atomic<bool> connected{false};
    std::atomic<bool> closing{false};
    std::unique_ptr<asio::steady_timer> pingTimer;
};

inline void to_json(Json& j, const Matchmaker::MessageHello& m) {
    j = Json{
        {"description", m.description},
    };
}

inline void from_json(const Json& j, Matchmaker::MessageHello& m) {
    j.at("description").get_to(m.description);
}

inline void to_json(Json& j, const Matchmaker::MessageServerRegister& m) {
    j = Json{
        {"name", m.name},
        {"endpoint", m.endpoint},
    };
}

inline void from_json(const Json& j, Matchmaker::MessageServerRegister& m) {
    j.at("name").get_to(m.name);
    j.at("endpoint").get_to(m.endpoint);
}

inline void to_json(Json& j, const Matchmaker::MessageServerRegistered& m) {
    j = Json{
        {"id", m.id},
    };
}

inline void from_json(const Json& j, Matchmaker::MessageServerRegistered& m) {
    j.at("id").get_to(m.id);
}

inline void to_json(Json& j, const Matchmaker::MessageServerPing& m) {
    j = Json{
        {"time", m.time},
    };
}

inline void from_json(const Json& j, Matchmaker::MessageServerPing& m) {
    j.at("time").get_to(m.time);
}
} // namespace Engine
