#include "../Utils/Json.hpp"
#include "NetworkUtils.hpp"
#include <vector>

namespace Engine {
class ENGINE_API NetworkMatchmaker {
public:
    NetworkMatchmaker(const Config& config);
    ~NetworkMatchmaker();

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
    void close();

    const Config& config;
    std::thread thread;
    asio::io_context service;
    std::unique_ptr<asio::io_service::work> work;
    asio::ip::tcp::resolver resolver;
    std::string host;
    std::array<uint8_t, 16> nonce{};
    std::unique_ptr<asio::ip::tcp::resolver::query> query;
    std::vector<asio::ip::tcp::endpoint> endpoints;
    asio::ssl::context ssl;
    asio::ssl::stream<asio::ip::tcp::socket> socket;
    asio::streambuf streambuf;
    std::array<uint8_t, 16 * 1024> buffer;
    std::vector<uint8_t> received;
};
} // namespace Engine
