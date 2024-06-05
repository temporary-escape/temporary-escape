#include "MatchmakerClient.hpp"
#include "../Network/NetworkUdpServer.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static const auto* matchmakerFileName = "online.xml";

MatchmakerClient::MatchmakerClient(const Config& config) : config{config}, url{config.network.matchmakerUrl} {
    api = std::make_shared<NetworkHttpsClient>(getService(), this->url);
    loadDataFile();
}

MatchmakerClient::~MatchmakerClient() {
    MatchmakerClient::close();
}

void MatchmakerClient::close() {
    if (api) {
        api->stop();
    }

    BackgroundWorker::stop();
    api.reset();
}

Future<MatchmakerClient::AuthMeResponse> MatchmakerClient::apiAuthGetMe() {
    auto promise = std::make_shared<Promise<AuthMeResponse>>();

    api->request(HttpMethod::Get, "/api/v1/auth/me", nullptr, [promise](const HttpResponse& res) {
        promise->resolve(AuthMeResponse{res});
    });

    return promise->future();
}

Future<MatchmakerClient::AuthStateCreatedResponse> MatchmakerClient::apiAuthCreateState() {
    auto promise = std::make_shared<Promise<AuthStateCreatedResponse>>();

    api->request(HttpMethod::Post, "/api/v1/auth/state", nullptr, [promise](const HttpResponse& res) {
        promise->resolve(AuthStateCreatedResponse{res});
    });

    return promise->future();
}

Future<MatchmakerClient::AuthLogInRespose> MatchmakerClient::apiAuthLogIn(const std::string& state) {
    AuthLogInRequest body{};
    body.state = state;

    auto promise = std::make_shared<Promise<AuthLogInRespose>>();

    api->request(HttpMethod::Post, "/api/v1/auth/login", body, [promise](const HttpResponse& res) {
        promise->resolve(AuthLogInRespose{res});
    });

    return promise->future();
}

Future<MatchmakerClient::ServerPageResponse> MatchmakerClient::apiServersList(const int page) {
    const auto path = fmt::format("/api/v1/servers?page={}", page);

    auto promise = std::make_shared<Promise<ServerPageResponse>>();

    api->request(HttpMethod::Get, path, nullptr, [promise](const HttpResponse& res) {
        promise->resolve(ServerPageResponse{res});
    });

    return promise->future();
}

void MatchmakerClient::apiServersRegister(std::string name, std::string version,
                                          std::function<void(ServerResponse)> callback) {
    ServerRegisterRequest body{};
    body.name = std::move(name);
    body.version = std::move(version);

    api->request(HttpMethod::Post, "/api/v1/servers", body, [c = std::move(callback)](const HttpResponse& res) {
        c(ServerResponse{res});
    });
}

void MatchmakerClient::apiServersUnregister(const std::string& serverId) {
    const auto path = fmt::format("/api/v1/servers/{}", serverId);
    api->request(HttpMethod::Delete, path, nullptr, [](const HttpResponse& res) {
        (void)res; // TODO
    });
}

std::string MatchmakerClient::getUrlForAuthRedirect(const std::string& state) const {
    return fmt::format("{}/token/{}", url, state);
}

bool MatchmakerClient::hasAuthorization() const {
    return api && !api->getAuthorization().empty();
}

const std::string& MatchmakerClient::getAuthorization() const {
    if (!api || api->getAuthorization().empty()) {
        EXCEPTION("Matchmaker has no authorization token");
    }
    return api->getAuthorization();
}

void MatchmakerClient::loadDataFile() {
    const auto path = config.userdataPath / matchmakerFileName;

    if (Fs::exists(path)) {
        try {
            MatchmakerDataFile file{};
            Xml::fromFile(path, file);
            api->setAuthorization(file.token);
        } catch (std::exception& e) {
            BACKTRACE(e, "Failed to load: {}", path);
        }
    }
}

void MatchmakerClient::saveDataFile() {
    const auto path = config.userdataPath / matchmakerFileName;

    try {
        MatchmakerDataFile file{};
        file.token = getAuthorization();
        Xml::toFile(path, file);
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to save: {}", path);
    }
}

/*void Matchmaker::onEvent(const EventConnectionRequest& event) {
    logger.info("Connection request event from: {} address: {} port: {}", event.id, event.address, event.port);

    if (udpServer) {
        udpServer->getStunClient().send([this, event](const NetworkStunClient::Result& stun) {
            udpServer->notifyClientConnection(event.address, event.port, [event, stun, w = ws]() {
                EventConnectionResponse data{};
                data.id = event.id;
                data.address = stun.endpoint.address().to_string();
                data.port = stun.endpoint.port();

                logger.info("Sending STUN result back for client: {}", event.id);
                w->send(Json{
                    {"event", EventConnectionResponse::type},
                    {"data", data},
                });
            });
        });
    }
}*/
