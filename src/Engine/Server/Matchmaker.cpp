#include "Matchmaker.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

Matchmaker::Matchmaker(Listener& listener, std::string url) : listener{listener}, url{std::move(url)} {
    addHandler<MessageHello>();
    addHandler<MessageServerRegistered>();

    ws = std::make_shared<NetworkWebsockets>(getService(), *this, this->url);
    ws->start();
}

Matchmaker::~Matchmaker() {
    Matchmaker::close();
}

void Matchmaker::serverRegister(std::string name, const asio::ip::udp::endpoint& endpoint) {
    logger.info("Matchmaker registering server...");

    Matchmaker::MessageServerRegister msg{};
    msg.name = std::move(name);
    msg.endpoint = endpoint;
    send(msg);
}

void Matchmaker::close() {
    closing.store(true);

    if (retry) {
        asio::error_code ec;
        (void)retry->cancel(ec);
    }
    if (pingTimer) {
        asio::error_code ec;
        (void)pingTimer->cancel(ec);
    }
    if (ws) {
        ws->stop();
    }

    BackgroundWorker::stop();
    ws.reset();
}

template <typename T> void Matchmaker::send(const T& value) {
    if (connected.load()) {
        ws->send(Json{
            {"type", std::string{T::type}},
            {"data", value},
        });
    }
}

void Matchmaker::onReceive(Json json) {
    try {
        if (!json.is_object() || !json.contains("type") || !json.at("type").is_string() || !json.contains("data") ||
            !json.at("data").is_object()) {
            EXCEPTION("Received message is not a valid JSON object");
        }

        const auto type = json["type"].get<std::string>();
        const auto it = handlers.find(type);
        if (it == handlers.end()) {
            EXCEPTION("No known handler for message type: {}", type);
        }

        it->second(json["data"]);
    } catch (std::exception& e) {
        BACKTRACE(e, "Matchmaker failed to handle message");
    }
}

void Matchmaker::retryWsConnection() {
    if (closing.load()) {
        return;
    }

    pingTimer.reset();

    retry = std::make_unique<asio::steady_timer>(getService(), std::chrono::seconds{10});
    retry->async_wait([this](const asio::error_code ec) {
        if (!ec) {
            ws = std::make_shared<NetworkWebsockets>(getService(), *this, this->url);
            ws->start();
        }
    });
}

void Matchmaker::doPing() {
    if (closing.load()) {
        return;
    }

    MessageServerPing msg{};
    msg.time = std::chrono::system_clock::now();
    send(msg);

    pingTimer = std::make_unique<asio::steady_timer>(getService(), std::chrono::seconds{10});
    pingTimer->async_wait([this](const asio::error_code ec) {
        if (ec) {
            logger.error("Matchmaker ping timer error: {}", ec.message());
        } else {
            doPing();
        }
    });
}

void Matchmaker::onConnect() {
    logger.info("Matchmaker connection connected");
    connected.store(true);
}

void Matchmaker::onConnectFailed() {
    logger.warn("Matchmaker connection failed, trying again...");
    connected.store(false);
    retryWsConnection();
}

void Matchmaker::onClose() {
    logger.warn("Matchmaker connection closed, reconnecting...");
    connected.store(false);
    listener.onMatchmakerDisconnect();
    retryWsConnection();
}

void Matchmaker::handle(const Matchmaker::MessageHello& msg) {
    logger.info("Matchmaker received hello from server: \"{}\"", msg.description);
    listener.onMatchmakerConnect();
}

void Matchmaker::handle(const Matchmaker::MessageServerRegistered& msg) {
    logger.info("Matchmaker received server registered with id: \"{}\"", msg.id);
    doPing();
}
