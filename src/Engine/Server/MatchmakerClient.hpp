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
class ENGINE_API MatchmakerClient : public BackgroundWorker {
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

    using ServerResponse = Response<Server>;

    using ServerPageResponse = Response<ServerPage>;

    struct ServerRegisterRequest {
        std::string name;
        std::string version;
    };

    MatchmakerClient(const Config& config);
    ~MatchmakerClient();

    void close();

    Future<AuthMeResponse> apiAuthGetMe();
    Future<AuthStateCreatedResponse> apiAuthCreateState();
    Future<AuthLogInRespose> apiAuthLogIn(const std::string& state);
    Future<ServerPageResponse> apiServersList(int page);
    void apiServersRegister(std::string name, std::string version, std::function<void(ServerResponse)> callback);
    void apiServersUnregister(const std::string& serverId);

    const std::string& getBaseUrl() const {
        return url;
    }

    bool hasAuthorization() const;

    const std::string& getAuthorization() const;

    std::string getUrlForAuthRedirect(const std::string& state) const;

    void saveDataFile();

private:
    void loadDataFile();

    const Config& config;
    std::string url;
    std::shared_ptr<NetworkHttpsClient> api;
};

template <> struct MatchmakerClient::Response<void> {
    Response(const HttpResponse& res) : error{res.error}, status{res.status} {
    }

    int status{0};
    std::string error;
};

inline void from_json(const Json& j, MatchmakerClient::AuthMe& m) {
    j.at("user_id").get_to(m.userId);
}

inline void from_json(const Json& j, MatchmakerClient::AuthStateCreated& m) {
    j.at("state").get_to(m.state);
}

inline void to_json(Json& j, const MatchmakerClient::AuthLogInRequest& m) {
    j = Json{
        {"state", m.state},
    };
}

inline void from_json(const Json& j, MatchmakerClient::Server& m) {
    j.at("id").get_to(m.id);
    j.at("name").get_to(m.name);
    j.at("version").get_to(m.version);
}

inline void to_json(Json& j, const MatchmakerClient::ServerRegisterRequest& m) {
    j = Json{
        {"name", m.name},
        {"version", m.version},
    };
}
} // namespace Engine

namespace nlohmann {
template <typename T> struct adl_serializer<Engine::MatchmakerClient::TypedPageModel<T>> {
    static void to_json(json& j, const Engine::MatchmakerClient::TypedPageModel<T>& m) {
        j = json{
            {"page", m.page},
            {"pages", m.pages},
            {"total", m.total},
            {"items", m.items},
        };
    }

    static void from_json(const json& j, Engine::MatchmakerClient::TypedPageModel<T>& m) {
        j.at("page").get_to(m.page);
        j.at("pages").get_to(m.pages);
        j.at("total").get_to(m.total);
        j.at("items").get_to(m.items);
    }
};
} // namespace nlohmann
