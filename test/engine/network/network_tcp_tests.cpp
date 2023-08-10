#include "../../common.hpp"
#include <engine/network/network_tcp_client.hpp>
#include <engine/network/network_tcp_server.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class TestNetworkDispatcher : public NetworkDispatcher {
public:
    void onAcceptSuccess(const NetworkPeerPtr& peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.push_back(peer);
    }

    void onDisconnect(const NetworkPeerPtr& peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.erase(std::remove(peers.begin(), peers.end(), peer), peers.end());
    }

    std::vector<NetworkPeerPtr> getPeers() {
        std::lock_guard<std::mutex> lock{mutex};
        return peers;
    }

private:
    std::mutex mutex;
    std::vector<NetworkPeerPtr> peers;
};

class TcpServerFixture {
public:
    TcpServerFixture() {
        work = std::make_unique<asio::io_service::work>(service);
        for (auto i = 0; i < 4; i++) {
            threads.emplace_back([this]() { service.run(); });
        }
    }

    ~TcpServerFixture() {
        stopAll();
    }

    void stopAll() {
        if (server) {
            logger.info("Closing server");
            server->stop();
            server.reset();
        }

        if (work) {
            logger.info("Stopping io_service");
            work.reset();
        }

        if (!threads.empty()) {
            logger.info("Joining threads");
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            threads.clear();
        }
    }

    void startServer() {
        server = std::make_unique<NetworkTcpServer>(service, dispatcher, 22334, true);
    }

    std::shared_ptr<NetworkTcpClient> createClient(NetworkDispatcher& d) {
        return std::make_shared<NetworkTcpClient>(service, d, "localhost", 22334);
    }

    PrivateKey pkey{};
    DiffieHellmanKey dh{};
    X509Cert cert{pkey};
    TestNetworkDispatcher dispatcher{};
    asio::io_service service;
    std::unique_ptr<asio::io_service::work> work;
    std::list<std::thread> threads;
    std::unique_ptr<NetworkTcpServer> server;
};

TEST_CASE_METHOD(TcpServerFixture, "Start and do nothing TCP server", "[tcp_server]") {
    startServer();
}

TEST_CASE_METHOD(TcpServerFixture, "Try to connect to no TCP server", "[tcp_server]") {
    NetworkDispatcher clientDispatcher{};
#ifdef _WIN32
    REQUIRE_THROWS_WITH(createClient(clientDispatcher),
                        "No connection could be made because the target machine actively refused it.");
#else
    REQUIRE_THROWS_WITH(createClient(clientDispatcher), "Connection refused");
#endif
}

TEST_CASE_METHOD(TcpServerFixture, "Start and close the TCP client", "[tcp_server]") {
    startServer();

    // Connect to the server
    NetworkDispatcher clientDispatcher{};
    auto client = createClient(clientDispatcher);
    REQUIRE(client->isConnected());
    REQUIRE(client->getAddress() == "[::1]:22334");
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);
}

TEST_CASE_METHOD(TcpServerFixture, "Start and close the TCP client many times", "[tcp_server]") {
    startServer();

    // Do the operation many times
    for (auto i = 0; i < 50; i++) {
        // Connect to the server
        NetworkDispatcher clientDispatcher{};
        auto client = createClient(clientDispatcher);
        REQUIRE(client->isConnected());
        REQUIRE(client->getAddress() == "[::1]:22334");

        REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

        // Close client connection
        client->close();
        REQUIRE(!client->isConnected());
        REQUIRE_EVENTUALLY(dispatcher.getPeers().empty());
    }
}

struct MyFooMsg {
    std::string msg;

    MSGPACK_DEFINE(msg);
};

MESSAGE_DEFINE(MyFooMsg);

TEST_CASE_METHOD(TcpServerFixture, "Send a message from the client to the server", "[tcp_server]") {
    std::atomic_bool received{false};
    MyFooMsg myFooMsg;

    dispatcher.addHandler([&](Request<MyFooMsg> request) {
        logger.info("Got MyFooMsg message!");
        myFooMsg = request.get();
        received.store(true);
    });

    startServer();

    // Connect to the server
    NetworkDispatcher clientDispatcher{};
    auto client = createClient(clientDispatcher);
    REQUIRE(client->isConnected());
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

    // Send a message
    MyFooMsg req{};
    req.msg = "Hello World!";
    client->send(req);

    // Wait for the message
    REQUIRE_EVENTUALLY(received.load());

    client->close();

    REQUIRE(myFooMsg.msg == req.msg);
}

TEST_CASE_METHOD(TcpServerFixture, "Send a message from the server to the client", "[tcp_server]") {
    startServer();

    NetworkDispatcher clientDispatcher{};

    std::atomic_bool received{false};
    MyFooMsg myFooMsg;

    clientDispatcher.addHandler([&](Request<MyFooMsg> request) {
        logger.info("Got MyFooMsg message!");
        myFooMsg = request.get();
        received.store(true);
    });

    // Connect to the server
    auto client = createClient(clientDispatcher);
    REQUIRE(client->isConnected());
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

    // Send a message
    MyFooMsg req{};
    req.msg = "Hello World!";
    dispatcher.getPeers().front()->send(req, 0);

    // Wait for the message
    REQUIRE_EVENTUALLY(received.load());

    client->close();

    REQUIRE(myFooMsg.msg == req.msg);
}

TEST_CASE_METHOD(TcpServerFixture, "Connect many clients to the server", "[tcp_server]") {
    std::mutex mutex;
    std::vector<MyFooMsg> received;

    dispatcher.addHandler([&](Request<MyFooMsg> request) {
        std::lock_guard<std::mutex> lock{mutex};
        received.push_back(request.get());
    });

    startServer();

    std::array<std::thread, 16> threads{};
    std::array<NetworkDispatcher, 16> clientDispatchers{};
    std::array<std::shared_ptr<NetworkTcpClient>, 16> clients{};

    for (auto i = 0; i < 16; i++) {
        threads[i] = std::thread([this, i, &clientDispatchers, &clients]() {
            clients[i] = createClient(clientDispatchers[i]);
            REQUIRE(clients[i]->isConnected());

            std::this_thread::sleep_for(std::chrono::milliseconds{500});

            MyFooMsg msg;
            msg.msg = fmt::format("Hello World from: {}", i);
            clients[i]->send(msg);

            std::this_thread::sleep_for(std::chrono::milliseconds{500});

            clients[i]->close();
        });
    }

    for (auto& thread : threads) {
        try {
            thread.join();
        } catch (std::exception& e) {
            logger.error(e.what());
            CHECK(false);
        }
    }

    stopAll();

    REQUIRE(received.size() == 16);
    for (auto i = 0; i < 16; i++) {
        const auto it = std::find_if(received.begin(), received.end(), [i](const MyFooMsg& e) {
            return e.msg == fmt::format("Hello World from: {}", i);
        });
        REQUIRE(it != received.end());
    }
}

struct DataRequest {
    uint8_t foo{0};

    MSGPACK_DEFINE(foo);
};

MESSAGE_DEFINE(DataRequest);

struct DataResponse {
    static constexpr size_t size = 1024 * 16;
    std::array<uint8_t, 1024 * 16> data;
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<DataResponse> {
        msgpack::object const& operator()(msgpack::object const& o, DataResponse& v) const {
            if (o.type != msgpack::type::BIN)
                throw msgpack::type_error();
            if (o.via.bin.size != v.data.size())
                throw msgpack::type_error();
            std::memcpy(v.data.data(), o.via.bin.ptr, v.data.size());
            return o;
        }
    };

    template <> struct pack<DataResponse> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, DataResponse const& v) const {
            o.pack_bin(v.data.size());
            o.pack_bin_body(reinterpret_cast<const char*>(v.data.data()), v.data.size());
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack

MESSAGE_DEFINE(DataResponse);

TEST_CASE_METHOD(TcpServerFixture, "Test for server throughput", "[tcp_server]") {
    std::array<uint8_t, DataResponse::size> randomData{};
    std::mt19937_64 rng{123456};
    std::uniform_int_distribution<int> dist{0, 255};
    for (auto& d : randomData) {
        d = dist(rng);
    }

    dispatcher.addHandler([&](Request<DataRequest> request) {
        (void)request.get();
        DataResponse res{};
        std::memcpy(res.data.data(), randomData.data(), randomData.size());
        request.respond(res);
    });

    startServer();

    size_t total{0};
    std::atomic_bool done{false};
    std::chrono::time_point<std::chrono::steady_clock> end{};

    NetworkDispatcher clientDispatcher{};
    clientDispatcher.addHandler([&](Request<DataResponse> request) {
        const auto r = request.get();
        total += r.data.size();
        if (total > 1024 * 1024 * 128) {
            done.store(true);
            end = std::chrono::steady_clock::now();
            return;
        }
        DataRequest res{};
        request.respond(res);
    });

    // Connect to the server
    auto client = createClient(clientDispatcher);
    REQUIRE(client->isConnected());
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

    const auto start = std::chrono::steady_clock::now();
    client->send(DataRequest{});

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds{30};
    while (timeout > std::chrono::steady_clock::now()) {
        if (done.load()) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds{1});
    }

    REQUIRE_EVENTUALLY_S(done.load(), 30);

    REQUIRE(done.load());
    client->close();

    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    logger.info("Throughput: {} MB/s", (static_cast<float>(total) / static_cast<float>(diff)) / 1000.0f);
}

struct MyFooRequest {
    int a{0};
    int b{0};

    MSGPACK_DEFINE(a, b);
};

MESSAGE_DEFINE(MyFooRequest);

struct MyFooResponse {
    int res{0};

    MSGPACK_DEFINE(res);
};

MESSAGE_DEFINE(MyFooResponse);

TEST_CASE_METHOD(TcpServerFixture, "Send a request to the server", "[tcp_server]") {
    dispatcher.addHandler([&](Request<MyFooRequest> request) {
        const auto data = request.get();
        MyFooResponse res;
        res.res = data.a * data.b;
        request.respond(res);
    });

    startServer();

    // Connect to the server
    NetworkDispatcher clientDispatcher{};
    auto client = createClient(clientDispatcher);
    REQUIRE(client->isConnected());
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

    MyFooRequest req{};
    req.a = 5;
    req.b = 10;
    auto promise = client->request<MyFooResponse>(req);
    auto future = promise->future();
    REQUIRE_WAIT_FOR(future, 2);

    auto res = future.get();
    REQUIRE(res.res == 50);
}

TEST_CASE_METHOD(TcpServerFixture, "Send many requests to the server from many clients", "[tcp_server]") {
    dispatcher.addHandler([&](Request<MyFooRequest> request) {
        const auto data = request.get();
        MyFooResponse res;
        res.res = data.a * data.b;
        request.respond(res);
    });

    startServer();

    std::array<std::thread, 16> threads{};
    std::array<NetworkDispatcher, 16> clientDispatchers{};
    std::array<std::shared_ptr<NetworkTcpClient>, 16> clients{};

    for (auto i = 0; i < 16; i++) {
        threads[i] = std::thread([this, i, &clientDispatchers, &clients]() {
            clients[i] = createClient(clientDispatchers[i]);
            REQUIRE(clients[i]->isConnected());

            std::this_thread::sleep_for(std::chrono::milliseconds{500});

            std::array<PromisePtr<MyFooResponse>, 32> promises{};
            for (auto x = 0; x < promises.size(); x++) {
                MyFooRequest req{};
                req.a = i;
                req.b = x;
                promises[x] = clients[i]->request<MyFooResponse>(req);
            }

            for (auto x = 0; x < promises.size(); x++) {
                auto future = promises[x]->future();
                REQUIRE_WAIT_FOR(future, 2);

                auto res = future.get();
                REQUIRE(res.res == i * x);
            }
        });
    }

    for (auto& thread : threads) {
        try {
            thread.join();
        } catch (std::exception& e) {
            logger.error(e.what());
            CHECK(false);
        }
    }
}

TEST_CASE_METHOD(TcpServerFixture, "Send a request to the server and receive error", "[tcp_server]") {
    dispatcher.addHandler([&](Request<MyFooRequest> request) {
        (void)request.get();
        request.respondError("This is some error");
    });

    startServer();

    // Connect to the server
    NetworkDispatcher clientDispatcher{};
    auto client = createClient(clientDispatcher);
    REQUIRE(client->isConnected());
    REQUIRE_EVENTUALLY(dispatcher.getPeers().size() == 1);

    MyFooRequest req{};
    req.a = 5;
    req.b = 10;
    auto promise = client->request<MyFooResponse>(req);
    auto future = promise->future();
    REQUIRE_WAIT_FOR(future, 2);
    REQUIRE_THROWS_WITH(future.get(), "This is some error");
}
