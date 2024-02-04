#include "../Future.hpp"
#include "../Utils/Json.hpp"
#include "NetworkUtils.hpp"
#include <vector>

namespace Engine {
class ENGINE_API NetworkWebsockets : public std::enable_shared_from_this<NetworkWebsockets> {
public:
    class Receiver {
    public:
        virtual ~Receiver() = default;

        virtual void onWsReceive(Json json) = 0;
        virtual void onWsConnect() = 0;
        virtual void onWsConnectFailed() = 0;
        virtual void onWsClose(int code) = 0;
    };

    NetworkWebsockets(asio::io_context& service, Receiver& receiver, std::string url, std::string token);
    virtual ~NetworkWebsockets();

    void start();
    void stop();
    void send(const Json& json);

private:
    struct Frame {
        enum class Opcode {
            None,
            Text,
            Close,
            Ping,
            Pong,
        };

        Opcode opcode{Opcode::None};
        std::string body;
        int code{0};

        operator bool() const {
            return opcode != Opcode::None;
        }
    };

    void send(Frame::Opcode opcode, const void* data, size_t length);
    void resolve();
    void connect();
    void handshake();
    void wsHandshakeSend();
    void wsHandshakeReceive();
    void receive();
    Frame tryDecode();
    void close(int code = 0);

    asio::io_context& service;
    Receiver& receiver;
    std::string url;
    std::string token;
    bool connected{false};
    asio::ip::tcp::resolver resolver;
    std::string host;
    std::string path;
    std::array<uint8_t, 16> nonce{};
    std::unique_ptr<asio::ip::tcp::resolver::query> query;
    std::vector<asio::ip::tcp::endpoint> endpoints;
    asio::ssl::context ssl;
    asio::ssl::stream<asio::ip::tcp::socket> socket;
    std::atomic<bool> shutdownFlag;
    asio::streambuf streambuf;
    std::array<uint8_t, 16 * 1024> buffer{};
    std::vector<uint8_t> received;
};
} // namespace Engine
