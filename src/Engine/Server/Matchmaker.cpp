#include "Matchmaker.hpp"
#include "../Network/NetworkUdpServer.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static std::chrono::seconds retryDelay{15};
static std::chrono::seconds pingDelay{10};
static const auto* matchmakerFileName = "online.xml";

Matchmaker::Matchmaker(const Config& config) : config{config}, url{config.network.matchmakerUrl} {
    api = std::make_shared<NetworkHttpsClient>(getService(), this->url);
    loadDataFile();
}

Matchmaker::~Matchmaker() {
    Matchmaker::close();
}

void Matchmaker::close() {
    stopFlag.store(true);

    if (retry) {
        asio::error_code ec;
        (void)retry->cancel(ec);
    }
    if (reconnect) {
        asio::error_code ec;
        (void)reconnect->cancel(ec);
    }
    if (api) {
        api->stop();
    }
    if (ws) {
        ws->stop();
    }

    BackgroundWorker::stop();
    api.reset();
    ws.reset();
    retry.reset();
    reconnect.reset();
}

Future<Matchmaker::AuthMeResponse> Matchmaker::apiAuthGetMe() {
    auto promise = std::make_shared<Promise<AuthMeResponse>>();

    api->request(HttpMethod::Get, "/api/v1/auth/me", nullptr, [promise](const HttpResponse& res) {
        promise->resolve(AuthMeResponse{res});
    });

    return promise->future();
}

Future<Matchmaker::AuthStateCreatedResponse> Matchmaker::apiAuthCreateState() {
    auto promise = std::make_shared<Promise<AuthStateCreatedResponse>>();

    api->request(HttpMethod::Post, "/api/v1/auth/state", nullptr, [promise](const HttpResponse& res) {
        promise->resolve(AuthStateCreatedResponse{res});
    });

    return promise->future();
}

Future<Matchmaker::AuthLogInRespose> Matchmaker::apiAuthLogIn(const std::string& state) {
    AuthLogInRequest body{};
    body.state = state;

    auto promise = std::make_shared<Promise<AuthLogInRespose>>();

    api->request(HttpMethod::Post, "/api/v1/auth/login", body, [promise](const HttpResponse& res) {
        promise->resolve(AuthLogInRespose{res});
    });

    return promise->future();
}

Future<Matchmaker::ServerPageResponse> Matchmaker::apiServersList(const int page) {
    const auto path = fmt::format("/api/v1/servers?page={}", page);

    auto promise = std::make_shared<Promise<ServerPageResponse>>();

    api->request(HttpMethod::Get, path, nullptr, [promise](const HttpResponse& res) {
        promise->resolve(ServerPageResponse{res});
    });

    return promise->future();
}

std::string Matchmaker::getUrlForAuthRedirect(const std::string& state) const {
    return fmt::format("{}/token/{}", url, state);
}

bool Matchmaker::hasAuthorization() const {
    return api && !api->getAuthorization().empty();
}

const std::string& Matchmaker::getAuthorization() const {
    if (!api || api->getAuthorization().empty()) {
        EXCEPTION("Matchmaker has no authorization token");
    }
    return api->getAuthorization();
}

/*void Matchmaker::apiAuthLogin(Callback<LoginResponse> callback) {
    static const auto path = "/api/v1/auth/login";

    LoginModel body{};
    body.username = uuid();
    body.password = "public";

    api->request(HttpMethod::Post, path, body, [this, c = std::move(callback)](const HttpResponse& res) {
        if (res.status == 201) {
            authorized.store(true);
        }
        c(LoginResponse{res});
    });
}*/

/*void Matchmaker::apiAuthLogout(Callback<LogoutResponse> callback) {
    static const auto path = "/api/v1/auth/logout";
    api->request(HttpMethod::Post, path, nullptr, [this, c = std::move(callback)](const HttpResponse& res) {
        if (res.status == 201) {
            authorized.store(false);
        }
        c(LogoutResponse{res});
    });
}*/

/*void Matchmaker::apiServersGet(const int page, Callback<ServerGetResponse> callback) {
    const auto path = fmt::format("/api/v1/servers?page={}", page);
    api->request(HttpMethod::Get, path, nullptr, [c = std::move(callback)](const HttpResponse& res) {
        c(ServerGetResponse{res});
    });
}*/

/*void Matchmaker::apiServersRegister(const RegisterServerModel& body, Callback<ServerRegisterResponse> callback) {
    static const auto path = "/api/v1/servers/register";
    api->request(HttpMethod::Post, path, body, [c = std::move(callback)](const HttpResponse& res) {
        c(ServerRegisterResponse{res});
    });
}*/

/*void Matchmaker::apiServersPing(const std::string& id, Callback<ServerPingResponse> callback) {
    const auto path = fmt::format("/api/v1/servers/{}/ping", id);
    api->request(HttpMethod::Put, path, nullptr, [c = std::move(callback)](const HttpResponse& res) {
        c(ServerPingResponse{res});
    });
}*/

/*void Matchmaker::apiServersConnect(const std::string& id, const ServerConnectModel& body,
                                   Matchmaker::Callback<Matchmaker::ServerConnectResponse> callback) {
    const auto path = fmt::format("/api/v1/servers/{}/connect", id);
    api->request(HttpMethod::Post, path, body, [c = std::move(callback)](const HttpResponse& res) {
        c(ServerConnectResponse{res});
    });
}*/

/*void Matchmaker::registerServerAndListen(std::string name, NetworkUdpServer& server) {
    post([this, n = std::move(name), s = &server]() {
        serverName = n;
        udpServer = s;
        doLogin();
    });
}*/

/*void Matchmaker::doLogin() {
    logger.info("Connecting to the matchmaker...");
    apiAuthLogin([this](const Matchmaker::LoginResponse& res) {
        if (!res.error.empty()) {
            logger.warn("Retrying login...");
            retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
            retry->async_wait([this](const asio::error_code ec) {
                if (!ec) {
                    doLogin();
                }
            });
        } else {
            doRegister();
        }
    });
}*/

/*void Matchmaker::doRegister() {
    logger.info("Registering server to the matchmaker...");

    Matchmaker::RegisterServerModel body{};
    body.name = serverName;
    body.version = GAME_VERSION;

    apiServersRegister(body, [this](const Matchmaker::ServerRegisterResponse& res) {
        // Bad login?
        if (!res.error.empty() && (res.status == 401 || res.status == 403)) {
            logger.warn("Retrying login...");
            retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
            retry->async_wait([this](const asio::error_code ec) {
                if (!ec) {
                    doLogin();
                }
            });
        }
        // Some generic error?
        else if (!res.error.empty()) {
            logger.warn("Retrying server registration...");
            retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
            retry->async_wait([this](const asio::error_code ec) {
                if (!ec) {
                    doRegister();
                }
            });
        }
        // Ok
        else {
            logger.info("Server registered with name: '{}' id: '{}'", res.data.name, res.data.id);
            serverName = res.data.name;
            serverId = res.data.id;
            doPing();
            startWs();
        }
    });
}*/

/*void Matchmaker::doPing() {
    apiServersPing(serverId, [this](const Matchmaker::ServerPingResponse& res) {
        // Bad login?
        if (!res.error.empty() && (res.status == 401 || res.status == 403)) {
            logger.warn("Retrying login...");
            retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
            retry->async_wait([this](const asio::error_code ec) {
                if (!ec) {
                    doLogin();
                }
            });
        }
        // Server was unregistered?
        else if (!res.error.empty() && res.status == 404) {
            logger.warn("Re-registering server to the matchmaker...");
            retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
            retry->async_wait([this](const asio::error_code ec) {
                if (!ec) {
                    doRegister();
                }
            });
        }
        // Ok or some generic error
        else {
            retry = std::make_unique<asio::steady_timer>(getService(), pingDelay);
            retry->async_wait([this](const asio::error_code ec) {
                if (!ec) {
                    doPing();
                }
            });
        }
    });
}*/

/*void Matchmaker::startWs() {
    if (stopFlag.load()) {
        return;
    }

    if (ws) {
        ws->stop();
    }
    ws.reset();

    const auto parts = parseUrl(url);
    auto addr = fmt::format("wss://{}:{}/api/servers/{}/events", parts->host, parts->port, serverId);
    ws = std::make_shared<NetworkWebsockets>(getService(), *this, addr, getAuthorization());
    ws->start();
}*/

void Matchmaker::onWsReceive(Json json) {
    // logger.info("Websockets received: {}", json.dump());
    try {
        if (json.is_object() && json.contains("event") && json.at("event").is_string()) {
            const auto event = json.at("event").get<std::string>();
            logger.info("Received event from the remote server type: {}", event);
            /*if (event == EventConnectionRequest::type) {
                post([this, json]() { onEvent(json.at("data").get<EventConnectionRequest>()); });
            }*/
        }
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to procerss websocket event");
    }
}

void Matchmaker::onWsConnect() {
    logger.info("Websockets conencted");
}

void Matchmaker::onWsConnectFailed() {
    logger.warn("Websockets connection failed");
    onWsClose(0);
}

void Matchmaker::onWsClose(const int code) {
    logger.error("Websockets close code: {}", code);
    if (code >= 3401 && code <= 3404) {
        logger.warn("Re-registering server to the matchmaker...");
        retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
        retry->async_wait([this](const asio::error_code ec) {
            if (!ec) {
                // doRegister();
            }
        });
    } else {
        logger.warn("Retrying websockets connection...");
        reconnect = std::make_unique<asio::steady_timer>(getService(), retryDelay);
        reconnect->async_wait([this](const asio::error_code ec) {
            if (!ec) {
                // startWs();
            }
        });
    }
}

void Matchmaker::loadDataFile() {
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

void Matchmaker::saveDataFile() {
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
