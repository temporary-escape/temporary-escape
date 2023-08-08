#include "../common.hpp"
#include <engine/network2/network_tcp_client.hpp>
#include <engine/network2/network_tcp_server.hpp>

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

class TestNetworkDispatcher : public NetworkDispatcher {
public:
    void onAcceptSuccess(const std::shared_ptr<NetworkTcpPeer>& peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.push_back(peer);
    }

    void onDisconnect(const std::shared_ptr<NetworkTcpPeer>& peer) override {
        std::lock_guard<std::mutex> lock{mutex};
        peers.erase(std::remove(peers.begin(), peers.end(), peer), peers.end());
    }

    std::vector<std::shared_ptr<NetworkTcpPeer>> getPeers() {
        std::lock_guard<std::mutex> lock{mutex};
        return peers;
    }

private:
    std::mutex mutex;
    std::vector<std::shared_ptr<NetworkTcpPeer>> peers;
};

class TcpServerFixture {
public:
    TcpServerFixture() {
        sslClient.setVerifyNone();
        sslServer.setPrivateKey(pkey);
        sslServer.setCertificate(cert);
        sslServer.setDiffieHellmanKey(dh);

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
        server = std::make_unique<NetworkTcpServer>(service, sslServer, dispatcher, 22334, true);
    }

    std::shared_ptr<NetworkTcpClient> createClient(NetworkDispatcher& d) {
        return std::make_shared<NetworkTcpClient>(service, sslClient, d, "localhost", 22334);
    }

    PrivateKey pkey{};
    DiffieHellmanKey dh{};
    X509Cert cert{pkey};
    NetworkSslContext sslClient{};
    NetworkSslContext sslServer{};
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
    client->send(req, 123);

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
    dispatcher.getPeers().front()->send(req, 123);

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

    std::list<std::future<void>> threads;
    for (auto i = 0; i < 16; i++) {
        threads.push_back(std::async([=]() {
            NetworkDispatcher clientDispatcher{};
            auto client = createClient(clientDispatcher);
            REQUIRE(client->isConnected());

            std::this_thread::sleep_for(std::chrono::milliseconds{500});

            MyFooMsg msg;
            msg.msg = fmt::format("Hello World from: {}", i);
            client->send(msg, 0);

            client->close();
        }));
    }

    for (auto& thread : threads) {
        REQUIRE_NOTHROW(thread.get());
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
    client->send(DataRequest{}, 123);

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds{5};
    while (timeout > std::chrono::steady_clock::now()) {
        if (done.load()) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds{30});
    }

    REQUIRE(done.load());
    client->close();

    const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    logger.info("Time took: {} ms bytes: {}", diff, total);
}
