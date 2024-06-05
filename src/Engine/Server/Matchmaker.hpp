#pragma once

#include "../Network/NetworkHttpsClient.hpp"
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
struct MatchmakerDataFile {
    std::string token;

    void convert(const Xml::Node& xml) {
        xml.convert("token", token);
    }

    void pack(Xml::Node& xml) const {
        xml.pack("token", token);
    }
};

XML_DEFINE(MatchmakerDataFile, "matchmaker");

class ENGINE_API NetworkUdpServer;
class ENGINE_API Matchmaker : public BackgroundWorker, public NetworkWebsockets::Receiver {
public:
    template <typename T> struct Response {
        Response(const HttpResponse& res) : error{res.error}, status{res.status} {
            if (status == 200) {
                data = res.body.get<T>();
            }
        }

        std::string error;
        int status{0};
        T data;
    };

    template <typename T> using Callback = std::function<void(const T&)>;

    struct PageModel {
        int page;
        int total;
        int pages;
    };

    template <typename T> struct TypedPageModel : PageModel {
        std::vector<T> items;
    };

    struct AuthMe {
        std::string userId;
    };

    using AuthMeResponse = Response<AuthMe>;

    struct AuthStateCreated {
        std::string state;
    };

    using AuthStateCreatedResponse = Response<AuthStateCreated>;

    struct AuthLogInRequest {
        std::string state;
    };

    using AuthLogInRespose = Response<void>;

    struct Server {
        std::string id;
        std::string name;
        std::string version;
    };

    using ServerPage = TypedPageModel<Server>;

    using ServerPageResponse = Response<ServerPage>;

    /*struct LoginModel {
        std::string username;
        std::string password;
    };

    struct PageModel {
        int page;
        int total;
        int pages;
    };

    struct RegisterServerModel {
        std::string name;
        std::string version;
    };

    struct ServerModel {
        std::string id;
        std::string name;
        std::string version;
    };

    struct ServerConnectModel {
        std::string address;
        int port;
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

    using ServerPageModel = TypedPageModel<ServerModel>;

    using LoginResponse = Response<void>;
    using LogoutResponse = Response<void>;
    using ServerGetResponse = Response<ServerPageModel>;
    using ServerRegisterResponse = Response<ServerModel>;
    using ServerPingResponse = Response<void>;
    using ServerConnectResponse = Response<ServerConnectModel>;*/

    Matchmaker(const Config& config);
    ~Matchmaker();

    void close();

    Future<AuthMeResponse> apiAuthGetMe();
    Future<AuthStateCreatedResponse> apiAuthCreateState();
    Future<AuthLogInRespose> apiAuthLogIn(const std::string& state);
    Future<ServerPageResponse> apiServersList(int page);

    /*void registerServerAndListen(std::string name, NetworkUdpServer& server);

    // void withLogin(std::function<void()> callback);
    void apiAuthLogin(Callback<LoginResponse> callback);
    void apiAuthLogout(Callback<LogoutResponse> callback);
    void apiServersGet(int page, Callback<ServerGetResponse> callback);
    void apiServersRegister(const RegisterServerModel& body, Callback<ServerRegisterResponse> callback);
    void apiServersPing(const std::string& id, Callback<ServerPingResponse> callback);
    void apiServersConnect(const std::string& id, const ServerConnectModel& body,
                           Callback<ServerConnectResponse> callback);
    void close();

    bool isAuthorized() const {
        return authorized.load();
    }
    const std::string& getUrl() const {
        return url;
    }
    const std::string& getAuthorization() const {
        static const std::string dummy;
        if (api) {
            return api->getAuthorization();
        }
        return dummy;
    }*/

    const std::string& getBaseUrl() const {
        return url;
    }

    bool hasAuthorization() const;

    const std::string& getAuthorization() const;

    std::string getUrlForAuthRedirect(const std::string& state) const;

    void saveDataFile();

private:
    void loadDataFile();
    // void doLogin();
    // void doRegister();
    // void doPing();
    /// void startWs();

    void onWsReceive(Json json) override;
    void onWsConnect() override;
    void onWsConnectFailed() override;
    void onWsClose(int code) override;

    // void onEvent(const EventConnectionRequest& event);

    const Config& config;
    std::string url;
    std::shared_ptr<NetworkHttpsClient> api;
    std::atomic<bool> authorized{false};
    std::atomic<bool> stopFlag{false};

    NetworkUdpServer* udpServer{nullptr};
    std::string serverName;
    std::string serverId;
    std::unique_ptr<asio::steady_timer> retry;
    std::shared_ptr<NetworkWebsockets> ws;
    std::unique_ptr<asio::steady_timer> reconnect;
};

template <> struct Matchmaker::Response<void> {
    Response(const HttpResponse& res) : error{res.error}, status{res.status} {
    }

    int status{0};
    std::string error;
};

/*inline void to_json(Json& j, const Matchmaker::LoginModel& m) {
    j = Json{
        {"username", m.username},
        {"password", m.password},
    };
}

inline void from_json(const Json& j, Matchmaker::LoginModel& m) {
    j.at("username").get_to(m.username);
    j.at("password").get_to(m.password);
}

inline void to_json(Json& j, const Matchmaker::RegisterServerModel& m) {
    j = Json{
        {"name", m.name},
        {"version", m.version},
    };
}

inline void from_json(const Json& j, Matchmaker::RegisterServerModel& m) {
    j.at("name").get_to(m.name);
    j.at("version").get_to(m.version);
}

inline void to_json(Json& j, const Matchmaker::ServerModel& m) {
    j = Json{
        {"id", m.id},
        {"name", m.name},
        {"version", m.version},
    };
}

inline void from_json(const Json& j, Matchmaker::ServerModel& m) {
    j.at("id").get_to(m.id);
    j.at("name").get_to(m.name);
    j.at("version").get_to(m.version);
}

inline void to_json(Json& j, const Matchmaker::ServerConnectModel& m) {
    j = Json{
        {"address", m.address},
        {"port", m.port},
    };
}

inline void from_json(const Json& j, Matchmaker::ServerConnectModel& m) {
    j.at("address").get_to(m.address);
    j.at("port").get_to(m.port);
}

inline void to_json(Json& j, const Matchmaker::EventConnectionRequest& m) {
    j = Json{
        {"id", m.id},
        {"address", m.address},
        {"port", m.port},
    };
}

inline void from_json(const Json& j, Matchmaker::EventConnectionRequest& m) {
    j.at("id").get_to(m.id);
    j.at("address").get_to(m.address);
    j.at("port").get_to(m.port);
}

inline void to_json(Json& j, const Matchmaker::EventConnectionResponse& m) {
    j = Json{
        {"id", m.id},
        {"address", m.address},
        {"port", m.port},
    };
}

inline void from_json(const Json& j, Matchmaker::EventConnectionResponse& m) {
    j.at("id").get_to(m.id);
    j.at("address").get_to(m.address);
    j.at("port").get_to(m.port);
}*/

inline void from_json(const Json& j, Matchmaker::AuthMe& m) {
    j.at("user_id").get_to(m.userId);
}

inline void from_json(const Json& j, Matchmaker::AuthStateCreated& m) {
    j.at("state").get_to(m.state);
}

inline void to_json(Json& j, const Matchmaker::AuthLogInRequest& m) {
    j = Json{
        {"state", m.state},
    };
}

inline void from_json(const Json& j, Matchmaker::Server& m) {
    j.at("id").get_to(m.id);
    j.at("name").get_to(m.name);
    j.at("version").get_to(m.version);
}
} // namespace Engine

namespace nlohmann {
template <typename T> struct adl_serializer<Engine::Matchmaker::TypedPageModel<T>> {
    static void to_json(json& j, const Engine::Matchmaker::TypedPageModel<T>& m) {
        j = json{
            {"page", m.page},
            {"pages", m.pages},
            {"total", m.total},
            {"items", m.items},
        };
    }

    static void from_json(const json& j, Engine::Matchmaker::TypedPageModel<T>& m) {
        j.at("page").get_to(m.page);
        j.at("pages").get_to(m.pages);
        j.at("total").get_to(m.total);
        j.at("items").get_to(m.items);
    }
};
} // namespace nlohmann
