#include "NetworkWebsockets.hpp"
#include "../Utils/Base64.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);

#define WS_FR_OP_TXT 1
#define WS_FR_OP_CLOSE 8
#define WS_FR_OP_PING 0x9
#define WS_FR_OP_PONG 0xA
#define WS_FIN 128
#define WS_MASK 128

struct Response {
    std::string protocol;
    int status{0};
    std::unordered_map<std::string, std::string> headers;

    bool hasHeader(const std::string& key, const std::string_view& value) const {
        const auto it = headers.find(key);
        if (it == headers.end()) {
            return false;
        }
        return it->second == value;
    }
};

static Response parseResponse(asio::streambuf& buff) {
    std::string_view raw{reinterpret_cast<const char*>(buff.data().data()), buff.data().size()};
    Response res{};

    size_t pos{0};
    size_t next;
    while ((next = raw.find("\r\n", pos)) != std::string::npos) {
        auto line = raw.substr(pos, next - pos);
        pos = next + 2;

        if (line.empty()) {
            break;
        }

        if (res.status == 0) {
            auto d = line.find(' ');
            if (d != std::string::npos) {
                res.protocol = line.substr(0, d);
            }
            d += 1;
            line = line.substr(d);

            d = line.find(' ');
            if (d != std::string::npos) {
                res.status = std::stoi(std::string{line.substr(0, d)});
            }
        } else {
            const auto d = line.find(": ");
            if (d != std::string::npos) {
                const auto key = line.substr(0, d);
                const auto value = line.substr(d + 2);
                res.headers.emplace(std::string{key}, std::string{value});
            }
        }
    }

    return res;
}

NetworkWebsockets::NetworkWebsockets(asio::io_context& service, Receiver& receiver, std::string url,
                                     std::string token) :
    service{service},
    receiver{receiver},
    token{std::move(token)},
    url{std::move(url)},
    resolver{service},
    ssl{asio::ssl::context::tls_client},
    socket{service, ssl} {

    // Ignore certificate
    socket.set_verify_mode(asio::ssl::verify_none);
}

NetworkWebsockets::~NetworkWebsockets() = default;

void NetworkWebsockets::start() {
    auto self = shared_from_this();
    service.post([self]() {
        self->connected = false;
        self->streambuf.consume(self->streambuf.size());
        self->received.clear();
        self->resolve();
    });
}

void NetworkWebsockets::stop() {
    if (!shutdownFlag.load()) {
        shutdownFlag.store(true);
        socket.async_shutdown([](const asio::error_code ec) {
            if (ec) {
                logger.error("WSS client stopping error: {}", ec.message());
            }
        });
    }
}

void NetworkWebsockets::resolve() {
    logger.info("WSS client connecting to: '{}'", url);

    const auto parts = parseUrl(url);
    if (!parts) {
        logger.error("WSS client invalid address: {}", url);
        receiver.onWsConnectFailed();
        return;
    }
    host = parts->host;
    path = parts->path;

    query =
        std::make_unique<asio::ip::tcp::resolver::query>(asio::ip::tcp::v4(), parts->host, std::to_string(parts->port));
    auto self = shared_from_this();

    using Result = asio::ip::tcp::resolver::results_type;
    resolver.async_resolve(*query, [self](const asio::error_code ec, const Result& result) {
        if (ec) {
            logger.error("WSS client failed to resolve query error: {}", ec.message());
            self->receiver.onWsConnectFailed();
        } else {
            self->endpoints.clear();
            for (const auto& e : result) {
                if (e.endpoint().address().is_v4()) {
                    logger.debug("WSS client resolved host: {}", e.endpoint());
                    self->endpoints.push_back(e.endpoint());
                }
            }

            self->connect();
        }
    });
}

void NetworkWebsockets::connect() {
    if (endpoints.empty()) {
        logger.error("WSS client failed to connect");
        receiver.onWsConnectFailed();
        return;
    }

    logger.debug("WSS client trying address: '{}'", endpoints.back());

    auto self = shared_from_this();
    socket.lowest_layer().async_connect(endpoints.back(), [self](const asio::error_code ec) {
        if (ec) {
            logger.warn("WSS client connect error: {}", ec.message());
            self->endpoints.pop_back();
            self->connect();
        } else {
            self->handshake();
        }
    });
}

void NetworkWebsockets::handshake() {
    auto self = shared_from_this();
    socket.async_handshake(asio::ssl::stream_base::client, [self](const asio::error_code ec) {
        if (ec) {
            logger.error("WSS client failed to perform handshake error: {}", ec.message());
            self->close();
        } else {
            logger.debug("WSS client connected");
            self->wsHandshakeSend();
        }
    });
}

void NetworkWebsockets::wsHandshakeSend() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<uint32_t> dist{0x00, 0xFF};
    for (auto& n : nonce) {
        n = dist(rng);
    }

    std::stringstream req;
    req << "GET " << path << " HTTP/1.1\r\n";
    req << "Host: " << host << "\r\n";
    req << "User-Agent: TemporaryEscape Engine\r\n";
    req << "Accept: */*\r\n";
    req << "Sec-Fetch-Dest: empty\r\n";
    req << "Sec-Fetch-Mode: websocket\r\n";
    req << "Sec-Fetch-Site: cross-site\r\n";
    req << "Connection: keep-alive, Upgrade\r\n";
    req << "Accept-Encoding: none\r\n";
    req << "Sec-WebSocket-Version: 13\r\n";
    req << "Sec-WebSocket-Key: " << base64Encode(nonce.data(), nonce.size()) << "\r\n";
    req << "Upgrade: websocket\r\n";
    req << "Origin: https://" << host << "\r\n";
    if (!token.empty()) {
        req << "Cookie: Authorization=Bearer " << token << "\r\n";
    }
    req << "\r\n";

    auto msg = std::make_shared<std::string>(req.str());
    auto buff = asio::buffer(msg->data(), msg->size());
    auto self = shared_from_this();
    socket.async_write_some(buff, [self](const asio::error_code ec, const size_t written) {
        if (ec) {
            logger.error("WSS client failed to send initial message error: {}", ec.message());
            self->close();
        } else {
            self->wsHandshakeReceive();
        }
    });
}

void NetworkWebsockets::wsHandshakeReceive() {
    auto self = shared_from_this();
    asio::async_read_until(socket, streambuf, "\r\n\r\n", [self](const asio::error_code ec, const std::size_t length) {
        if (ec) {
            logger.error("WSS client failed to read initial message error: {}", ec.message());
            self->close();
        } else {
            self->streambuf.commit(length);
            const auto res = parseResponse(self->streambuf);
            if (res.status != 101 || !res.hasHeader("Upgrade", "websocket")) {
                logger.error("WSS client failed to perform handshake status: {}", res.status);
                self->close();
            } else {
                logger.info("WSS client received handshake");

                // Start receiving
                self->receive();
                self->receiver.onWsConnect();

                self->connected = true;
            }
            self->streambuf.consume(length);
        }
    });
}

void NetworkWebsockets::send(const Json& json) {
    auto payload = json.dump();
    send(Frame::Opcode::Text, payload.data(), payload.size());
}

void NetworkWebsockets::send(const Frame::Opcode opcode, const void* data, const size_t length) {
    auto msg = std::make_shared<std::vector<uint8_t>>();

    uint8_t op;
    switch (opcode) {
    case Frame::Opcode::Text: {
        op = WS_FR_OP_TXT;
        break;
    }
    case Frame::Opcode::Pong: {
        op = WS_FR_OP_PONG;
        break;
    }
    default: {
        EXCEPTION("Unable to send message due to invalid frame opcode");
    }
    }

    size_t header;
    if (length <= 125) {
        header = 2;
        msg->resize(header + 4 + length);
        auto* frame = msg->data();
        frame[0] = WS_FIN | op;
        frame[1] = WS_MASK | (length & 0x7F);
    } else if (length <= 65535) {
        header = 4;
        msg->resize(header + 4 + length);
        auto* frame = msg->data();
        frame[0] = WS_FIN | op;
        frame[1] = WS_MASK | 126;
        frame[2] = (length >> 8) & 0xFF;
        frame[3] = length & 0xFF;
    } else {
        header = 10;
        msg->resize(header + 4 + length);
        auto* frame = msg->data();
        frame[0] = WS_FIN | op;
        frame[1] = WS_MASK | 127;
        frame[2] = (unsigned char)((length >> 56) & 0xFF);
        frame[3] = (unsigned char)((length >> 48) & 0xFF);
        frame[4] = (unsigned char)((length >> 40) & 0xFF);
        frame[5] = (unsigned char)((length >> 32) & 0xFF);
        frame[6] = (unsigned char)((length >> 24) & 0xFF);
        frame[7] = (unsigned char)((length >> 16) & 0xFF);
        frame[8] = (unsigned char)((length >> 8) & 0xFF);
        frame[9] = (unsigned char)(length & 0xFF);
    }

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<uint32_t> dist{0x00, 0xFF};
    std::array<uint8_t, 4> mask{};
    for (auto& n : mask) {
        n = dist(rng);
    }

    std::memcpy(msg->data() + header, mask.data(), mask.size());

    auto* dst = msg->data() + header + 4;
    std::memcpy(dst, data, length);
    for (size_t i = 0; i < length; i++) {
        dst[i] = dst[i] ^ mask[i % 4];
    }

    auto buff = asio::buffer(msg->data(), msg->size());
    auto self = shared_from_this();
    socket.async_write_some(buff, [self](const asio::error_code ec, const std::size_t written) {
        if (ec) {
            logger.error("WSS client failed to send message error: {}", ec.message());
            self->close();
        }
    });
}

void NetworkWebsockets::close(const int code) {
    if (shutdownFlag.load()) {
        return;
    }

    shutdownFlag.store(true);
    socket.async_shutdown([](const asio::error_code ec) {
        if (ec) {
            logger.error("WSS client closing connection error: {}", ec.message());
        }
    });

    if (connected) {
        receiver.onWsClose(code);
    } else {
        receiver.onWsConnectFailed();
    }

    connected = false;
}

void NetworkWebsockets::receive() {
    auto buff = asio::buffer(buffer.data(), buffer.size());
    auto self = shared_from_this();
    socket.async_read_some(buff, [self](const asio::error_code ec, const std::size_t read) {
        if (ec) {
            logger.error("WSS client failed to read message error: {}", ec.message());
            self->close();
        } else {
            const auto offset = self->received.size();
            self->received.resize(offset + read);
            std::memcpy(self->received.data() + offset, self->buffer.data(), read);

            try {
                while (true) {
                    const auto frame = self->tryDecode();
                    if (frame.opcode == Frame::Opcode::Text) {
                        Json json;
                        try {
                            json = Json::parse(frame.body);
                        } catch (std::exception& e) {
                            logger.error("Received malformed json error: {}", e.what());
                        }
                        self->receiver.onWsReceive(std::move(json));
                    } else if (frame.opcode == Frame::Opcode::Close) {
                        logger.debug("WSS client received connection closed frame");
                        self->close(frame.code);
                        return;
                    } else if (frame.opcode == Frame::Opcode::Ping) {
                        self->send(Frame::Opcode::Pong, frame.body.data(), frame.body.size());
                    } else {
                        break;
                    }
                }

                self->receive();
            } catch (std::exception& e) {
                BACKTRACE(e, "WSS client failed to parse message");
                self->close();
            }
        }
    });
}

NetworkWebsockets::Frame NetworkWebsockets::tryDecode() {
    const uint8_t* src = received.data();
    auto read = received.size();

    if (read < 2) {
        return {};
    }

    if ((src[0] & WS_FIN) == 0) {
        EXCEPTION("Not a FIN frame");
    }

    Frame::Opcode opcode;
    if ((src[0] & WS_FR_OP_PING) == WS_FR_OP_PING) {
        opcode = Frame::Opcode::Ping;
    } else if ((src[0] & WS_FR_OP_PONG) == WS_FR_OP_PONG) {
        opcode = Frame::Opcode::Pong;
    } else if (src[0] & WS_FR_OP_CLOSE) {
        opcode = Frame::Opcode::Close;
    } else if (src[0] & WS_FR_OP_TXT) {
        opcode = Frame::Opcode::Text;
    } else {
        EXCEPTION("Invalid frame opcode");
    }

    const auto masked = (src[1] & WS_MASK) != 0;
    size_t length = src[1] & 0x7F;

    read -= 2;
    src += 2;

    int code{0};

    if (opcode == Frame::Opcode::Close) {
        if (read < 2) {
            return {};
        }

        code = *reinterpret_cast<const uint16_t*>(src);
        code = ((code & 0xFF00) >> 8) | ((code & 0x00FF) << 8);

        src += 2;
    }

    Frame frame{};
    frame.opcode = opcode;
    frame.code = code;

    if (opcode != Frame::Opcode::Close) {
        // More header data?
        if (length == 126) {
            if (read < 2) {
                return {};
            }

            length = 0;
            length |= static_cast<size_t>(src[2]) << 8;
            length |= static_cast<size_t>(src[3]) << 0;

            read -= 2;
            src += 2;
        }
        // Or even more header data?
        else if (length == 127) {
            if (read < 8) {
                return {};
            }

            length = 0;
            length |= static_cast<size_t>(src[2]) << 56;
            length |= static_cast<size_t>(src[3]) << 48;
            length |= static_cast<size_t>(src[4]) << 40;
            length |= static_cast<size_t>(src[5]) << 32;
            length |= static_cast<size_t>(src[6]) << 24;
            length |= static_cast<size_t>(src[7]) << 16;
            length |= static_cast<size_t>(src[8]) << 8;
            length |= static_cast<size_t>(src[9]) << 0;

            read -= 8;
            src += 8;
        }

        // Mask?
        const uint8_t* mask{nullptr};
        if (masked) {
            if (read < 4) {
                return {};
            }
            mask = src;
            read -= 4;
            src += 4;
        }

        // Do we have enough bytes?
        if (read < length) {
            return {};
        }

        frame.body.resize(length);
        std::memcpy(frame.body.data(), src, length);
        src += length;

        if (mask) {
            for (size_t i = 0; i < frame.body.size(); i++) {
                frame.body[i] = static_cast<char>(static_cast<uint8_t>(frame.body[i]) ^ mask[i % 4]);
            }
        }
    }

    const auto consumed = src - received.data();
    const auto leftover = received.size() - consumed;
    for (size_t i = 0; i < leftover; i++) {
        received[i] = src[i];
    }
    received.resize(leftover);

    return frame;
}
