#include "Client.hpp"
#include "../Network/NetworkTcpClient.hpp"
#include "../Server/Messages.hpp"
#include "../Utils/Random.hpp"
#include <fstream>

#define CMP "Client"

#define DISPATCH_FUNC(M, T, F) std::bind(static_cast<void (T::*)(M)>(&T::F), this, std::placeholders::_1)
#define MESSAGE_DISPATCH(M) dispatcher.add<M>(DISPATCH_FUNC(M, Client, handle));

using namespace Scissio;

std::atomic<uint64_t> Client::nextRequestId(1);

uint64_t generatePlayerUid(const Config& config) {
    const auto path = config.userdataPath / Path("uid");
    if (Fs::exists(path)) {
        std::ifstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for reading", path.string());
        }

        const std::string str((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

        return std::stoull(str);
    } else {
        const auto uid = randomId();

        std::ofstream f(path);
        if (!f.is_open()) {
            EXCEPTION("Failed to open '{}' for writing", path.string());
        }

        f << uid;

        return uid;
    }
}

Client::Client(Config& config, const std::string& address, const int port)
    : listener(std::make_unique<EventListener>(*this)),
      network(std::make_shared<Network::TcpClient>(*listener, address, port)), secret(generatePlayerUid(config)) {

    MESSAGE_DISPATCH(MessageLoginResponse);
    MESSAGE_DISPATCH(MessagePingResponse);
    MESSAGE_DISPATCH(MessageLatencyResponse);
    MESSAGE_DISPATCH(MessageFetchResponse<SystemData>);

    auto future = connectPromise.future();
    if (future.waitFor(std::chrono::milliseconds(1000)) != std::future_status::ready) {
        EXCEPTION("Timeout while waiting for server connection");
    }
    future.get();
}

Client::~Client() {
}

void Client::eventPacket(Network::Packet packet) {
    try {
        stats.packetsReceived++;
        dispatcher.dispatch(packet);
    } catch (...) {
        EXCEPTION_NESTED("Failed to dispatch message");
    }
}

void Client::eventConnect() {
    connectPromise.resolve();

    network->send(MessagePingRequest{std::chrono::system_clock::now()});
    network->send(MessageLatencyRequest{std::chrono::system_clock::now()});
}

void Client::eventDisconnect() {
}

Future<void> Client::login(const std::string& password) {
    loginPromise = Promise<void>{};

    MessageLoginRequest req{};
    req.secret = secret;
    req.name = "Some Player";
    req.password = password;

    network->send(req);

    return loginPromise.future();
}

void Client::update() {
    sync.restart();
    sync.run();
}

void Client::handle(MessageLoginResponse res) {
    if (!res.error.empty()) {
        loginPromise.reject<std::runtime_error>(res.error);
    } else {
        playerId = res.playerId;
        loginPromise.resolve();
    }
}

void Client::handle(MessagePingResponse res) {
    const auto now = std::chrono::system_clock::now();
    const auto diff = now - res.timePoint;
    stats.serverLatencyMs.store(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());

    Log::d(CMP, "Network latency: {} ms", stats.serverLatencyMs.load());

    worker1s.post([this]() { network->send(MessagePingRequest{std::chrono::system_clock::now()}); });
}

void Client::handle(MessageLatencyResponse res) {
    const auto now = std::chrono::system_clock::now();
    const auto diff = now - res.timePoint;
    stats.serverLatencyMs.store(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());

    Log::d(CMP, "Server latency: {} ms", stats.serverLatencyMs.load());

    worker1s.post([this]() { network->send(MessageLatencyRequest{std::chrono::system_clock::now()}); });
}

template <typename T> void Client::handle(MessageFetchResponse<T> res) {
    if (res.id) {
        AbstractRequestPtr abstractReq = nullptr;

        {
            std::lock_guard<std::mutex> lock{requestsMutex};
            const auto it = requests.find(res.id);

            if (it == requests.end()) {
                Log::e(CMP, "Got response for unknown fetch request id: {} type: {}", res.id, typeid(T).name());
            } else {
                abstractReq = it->second;
            }
        }

        if (!abstractReq) {
            return;
        }

        const auto req = std::dynamic_pointer_cast<Request<T>>(abstractReq);
        if (!req) {
            Log::e(CMP, "Got response for invalid type fetch request id: {} type: {}", res.id, typeid(T).name());
            return;
        }

        // Log::d(CMP, "Request id: {} received {} items", res.id, res.data.size());

        if (res.data.empty()) {
            // Done!

            // Log::d(CMP, "Syncing req: {}", req->getId());

            sync.post([this, req]() {
                // Log::d(CMP, "Completing req: {}", req->getId());

                {
                    std::lock_guard<std::mutex> lock{requestsMutex};
                    requests.erase(req->getId());
                }

                req->complete();
            });

        } else {
            // Not done yet, needs more data!

            req->append(res.data);

            MessageFetchRequest<T> req;
            req.id = res.id;
            req.prefix = res.prefix;
            req.start = res.next;

            network->send(req);
        }
    }
}
