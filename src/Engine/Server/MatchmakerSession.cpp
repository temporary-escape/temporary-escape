#include "MatchmakerSession.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

static std::chrono::seconds retryDelay{15};
static std::chrono::seconds pingDelay{10};

MatchmakerSession::MatchmakerSession(MatchmakerClient& client, Receiver& receiver, std::string serverName) :
    client{client}, receiver{receiver}, serverName{std::move(serverName)}, url{client.getBaseUrl()} {
    doRegister();
}

MatchmakerSession::~MatchmakerSession() {
    MatchmakerSession::close();
}

void MatchmakerSession::sendStunResponse(const std::string& id, const asio::ip::udp::endpoint& endpoint) {
    post([this, id, endpoint]() {
        EventConnectionResponse data{};
        data.id = id;
        data.address = endpoint.address().to_string();
        data.port = endpoint.port();

        logger.info("Sending STUN result back for client: {}", id);
        ws->send(Json{
            {"event", EventConnectionResponse::type},
            {"data", data},
        });
    });
}

void MatchmakerSession::close() {
    if (!serverId.empty()) {
        client.apiServersUnregister(serverId);
    }

    stopFlag.store(true);

    asio::error_code ec;
    if (ping) {
        (void)ping->cancel(ec);
    }
    if (retry) {
        (void)retry->cancel(ec);
    }
    if (reconnect) {
        (void)reconnect->cancel(ec);
    }
    if (ws) {
        ws->stop();
    }

    BackgroundWorker::stop();
    ping.reset();
    ws.reset();
    retry.reset();
    reconnect.reset();
}

void MatchmakerSession::doPing() {
    EventStatus data{};
    data.numPlayers = 0;

    ws->send(Json{
        {"event", EventStatus::type},
        {"data", data},
    });

    startPing();
}

void MatchmakerSession::doRegister() {
    client.apiServersRegister(serverName, GAME_VERSION, [this](MatchmakerClient::ServerResponse resp) {
        if (!resp.error.empty()) {
            logger.error("Failed to register server error: {}", resp.error);
        } else if (resp.status != 200) {
            logger.error("Failed to register server responded with code: {}", resp.status);
        } else {
            logger.info("Server registered with id: {}", resp.data.id);
            postSafe([this, id = resp.data.id]() {
                serverId = id;
                startWs();
            });
        }
    });
}

void MatchmakerSession::startWs() {
    if (stopFlag.load()) {
        return;
    }

    if (ws) {
        ws->stop();
    }
    ws.reset();

    const auto parts = parseUrl(url);
    auto addr = fmt::format("wss://{}:{}/api/v1/servers/{}/events", parts->host, parts->port, serverId);
    ws = std::make_shared<NetworkWebsockets>(getService(), *this, addr, client.getAuthorization());
    ws->start();
}

void MatchmakerSession::onWsReceive(Json json) {
    // logger.info("Websockets received: {}", json.dump());
    try {
        if (json.is_object() && json.contains("event") && json.at("event").is_string()) {
            const auto event = json.at("event").get<std::string>();
            logger.info("Received event from the remote server type: {}", event);
            if (event == EventConnectionRequest::type) {
                post([this, json]() { receiver.onStunRequest(json.at("data").get<EventConnectionRequest>()); });
            }
        }
    } catch (std::exception& e) {
        BACKTRACE(e, "Failed to process websocket event");
    }
}

void MatchmakerSession::onWsConnect() {
    logger.info("Websockets connected");

    startPing();
}

void MatchmakerSession::startPing() {
    ping = std::make_unique<asio::steady_timer>(getService(), pingDelay);
    ping->async_wait([this](const asio::error_code ec) {
        if (!ec) {
            doPing();
        }
    });
}

void MatchmakerSession::onWsConnectFailed() {
    logger.warn("Websockets connection failed");
    onWsClose(0);
}

void MatchmakerSession::onWsClose(const int code) {
    logger.error("Websockets close code: {}", code);

    asio::error_code ec;
    if (ping) {
        (void)ping->cancel(ec);
    }

    if (code >= 3401 && code <= 3404) {
        logger.warn("Re-registering server to the matchmaker...");
        retry = std::make_unique<asio::steady_timer>(getService(), retryDelay);
        retry->async_wait([this](const asio::error_code ec) {
            if (!ec) {
                doRegister();
            }
        });
    } else {
        logger.warn("Retrying websockets connection...");
        reconnect = std::make_unique<asio::steady_timer>(getService(), retryDelay);
        reconnect->async_wait([this](const asio::error_code ec) {
            if (!ec) {
                startWs();
            }
        });
    }
}
