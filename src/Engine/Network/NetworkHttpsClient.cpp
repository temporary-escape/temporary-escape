#include "NetworkHttpsClient.hpp"
#include "../Utils/StringUtils.hpp"

using namespace Engine;

static auto logger = createLogger(LOG_FILENAME);
static constexpr size_t readChunkSize = 64;

static std::string_view methodToStr(const HttpMethod method) {
    switch (method) {
    case HttpMethod::Get: {
        return "GET";
    }
    case HttpMethod::Post: {
        return "POST";
    }
    case HttpMethod::Put: {
        return "PUT";
    }
    case HttpMethod::Delete: {
        return "DELETE";
    }
    default: {
        EXCEPTION("Unknown HTTP method");
    }
    }
}

static size_t parseResponse(const std::string_view& raw, HttpResponse& res) {
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
                // logger.info("HEADER: {}: {}", key, value);
                res.headers.emplace(toLower(key), std::string{value});
            }
        }
    }

    return pos;
}

std::string HttpResponse::formatError() const {
    if (!error.empty()) {
        return error;
    }

    return fmt::format("Server returned status code: {}", status);
}

NetworkHttpsClient::NetworkHttpsClient(asio::io_context& service, std::string url) :
    service{service}, strand{service}, url{std::move(url)}, resolver{service}, ssl{asio::ssl::context::tls_client} {
}

NetworkHttpsClient::~NetworkHttpsClient() = default;

void NetworkHttpsClient::stop() {
    auto self = shared_from_this();
    /*strand.post([self]() {
        if (self->socket) {
            asio::error_code ec;
            (void)self->socket->shutdown(ec);
            if (ec) {
                logger.warn("HTTPS client shutdown error: {}", ec.message());
            }
        }
    });*/
}

void NetworkHttpsClient::request(const HttpMethod method, std::string path, Json json, HttpResponseCallback callback) {
    logger.debug("HTTPS client {} {}{}", methodToStr(method), url, path);

    auto req = std::make_shared<HttpRequest>(service, ssl);
    req->callback = std::move(callback);
    req->path = std::move(path);
    req->method = method;
    if (!json.empty() && !json.is_null()) {
        req->json = std::move(json);
    }

    auto self = shared_from_this();
    strand.post([self, req]() { self->resolve(req); });
}

void NetworkHttpsClient::errored(const NetworkHttpsClient::HttpRequestPtr& req, const std::string_view& msg) {
    logger.error("HTTPS client {} {}{} HTTP/1.1 - ({})", methodToStr(req->method), url, req->path, msg);
    req->callback(HttpResponse{0, std::string{msg}});
}

void NetworkHttpsClient::resolve(const HttpRequestPtr& req) {
    logger.debug("HTTPS client resolving address: '{}'", url);

    const auto parts = parseUrl(url);
    if (!parts) {
        logger.error("HTTPS client invalid host: {}", url);
        errored(req, "Invalid server URL");
        return;
    }
    host = parts->host;

    query =
        std::make_unique<asio::ip::tcp::resolver::query>(asio::ip::tcp::v4(), parts->host, std::to_string(parts->port));
    auto self = shared_from_this();

    using Result = asio::ip::tcp::resolver::results_type;
    resolver.async_resolve(*query, strand.wrap([self, req](const asio::error_code ec, const Result& result) {
        if (ec) {
            logger.error("HTTPS client failed to resolve query error: {}", ec.message());
            self->errored(req, ec.message());
        } else {
            self->endpoint = *result.begin();
            logger.debug("HTTPS client resolved host to: {}", self->endpoint);
            self->connect(req);
        }
    }));
}

void NetworkHttpsClient::connect(const HttpRequestPtr& req) {
    // Ignore certificate
    req->socket.set_verify_mode(asio::ssl::verify_none);

    auto self = shared_from_this();
    req->socket.lowest_layer().async_connect(endpoint, strand.wrap([self, req](const asio::error_code ec) {
        if (ec) {
            logger.warn("HTTPS client connect error: {}", ec.message());
            self->errored(req, ec.message());
        } else {
            self->handshake(req);
        }
    }));
}

void NetworkHttpsClient::handshake(const HttpRequestPtr& req) {
    auto self = shared_from_this();
    req->socket.async_handshake(asio::ssl::stream_base::client, strand.wrap([self, req](asio::error_code ec) {
        if (ec) {
            logger.error("HTTPS client failed to perform handshake error: {}", ec.message());
            (void)req->socket.shutdown(ec);
        } else {
            self->send(req);
        }
    }));
}

void NetworkHttpsClient::send(const HttpRequestPtr& req) {
    std::string body;
    if (!req->json.empty() && !req->json.is_null()) {
        body = req->json.dump();
    }

    std::stringstream ss;
    switch (req->method) {
    case HttpMethod::Post: {
        ss << "POST ";
        break;
    }
    case HttpMethod::Put: {
        ss << "PUT ";
        break;
    }
    case HttpMethod::Delete: {
        ss << "DELETE ";
        break;
    }
    default: {
        ss << "GET ";
        break;
    }
    }
    ss << req->path << " HTTP/1.1\r\n";
    ss << "Host: " << host << "\r\n";
    ss << "User-Agent: TemporaryEscape Engine\r\n";
    ss << "Accept: application/json\r\n";
    ss << "Origin: " << url << "\r\n";
    if (!token.empty()) {
        ss << "Cookie: Authorization=Bearer " << token << "\r\n";
    }
    if (!body.empty()) {
        ss << "Content-Type: application/json\r\n";
        ss << "Content-Length: " << body.size() << "\r\n";
    }
    ss << "\r\n";
    if (!body.empty()) {
        ss << body;
    }
    req->buffer = ss.str();

    auto buff = asio::buffer(req->buffer.data(), req->buffer.size());
    auto self = shared_from_this();
    req->socket.async_write_some(buff, strand.wrap([self, req](asio::error_code ec, const size_t written) {
        (void)written;
        if (ec) {
            logger.error("HTTPS client failed to send request error: {}", ec.message());
            (void)req->socket.shutdown(ec);
        } else {
            req->buffer.clear();
            self->read(req);
        }
    }));
}

void NetworkHttpsClient::read(const NetworkHttpsClient::HttpRequestPtr& req) {
    auto toRead = readChunkSize;
    if (req->contentLength && req->consumed + toRead > req->contentLength) {
        toRead = req->contentLength - req->consumed;
    }

    if (req->consumed + toRead > req->buffer.size()) {
        req->buffer.resize(req->consumed + toRead);
    }

    auto buff = asio::buffer(req->buffer.data() + req->consumed, toRead);
    auto self = shared_from_this();

    req->socket.async_read_some(buff, strand.wrap([self, req](asio::error_code ec, const std::size_t read) {
        if (ec) {
            logger.error("HTTPS client failed to read response body error: {}", ec.message());
        } else {
            req->consumed += read;

            // Are we reading the body?
            if (req->contentLength) {
                // We have the entire response body
                if (req->consumed == req->contentLength) {
                    self->processBody(req);
                }
                // We need more bytes
                else {
                    self->read(req);
                }
                return;
            }

            // Did we get the header?
            const auto pos = req->buffer.find("\r\n\r\n");
            if (pos != std::string::npos) {
                const auto headerLength = parseResponse(req->buffer, req->res);

                const auto leftover = req->consumed - headerLength;
                for (size_t i = 0; i < leftover; i++) {
                    req->buffer[i] = req->buffer[i + headerLength];
                }
                req->buffer.resize(leftover);

                const auto cl = req->res.headers.find("content-length");
                if (cl != req->res.headers.end()) {
                    req->contentLength = std::stoll(cl->second);
                }

                const auto sc = req->res.headers.find("set-cookie");
                if (sc != req->res.headers.end()) {
                    self->processCookies(sc->second);
                }

                if (req->contentLength) {
                    req->consumed = leftover;

                    self->read(req);
                    return;
                }

                (void)req->socket.shutdown(ec);
                self->processBody(req);
                return;
            }
            // No header yet, read more
            else {
                self->read(req);
            }
        }
    }));
}

void NetworkHttpsClient::processCookies(const std::string_view& header) {
    static const std::string_view bearer = "Bearer ";

    const auto tokens = split(header, ";");

    if (startsWith(tokens[0], "Authorization")) {
        const auto pos = tokens[0].find(bearer);
        if (pos != std::string::npos) {
            token = tokens[0].substr(pos + bearer.size());
            if (!token.empty() && token.back() == '\"') {
                token = {token.data(), token.size() - 1};
            }
        } else {
            token = "";
        }
    }
}

void NetworkHttpsClient::setAuthorization(const std::string& value) {
    token = value;
}

void NetworkHttpsClient::processBody(const NetworkHttpsClient::HttpRequestPtr& req) {
    asio::error_code ec;
    (void)req->socket.shutdown(ec);

    // No response body?
    if (req->buffer.empty()) {
        // Should we have had a body?
        if (req->contentLength) {
            req->res.error = "Bad response body";
            finalize(req);
            return;
        }

        finalize(req);
        return;
    }

    // Parse the body
    try {
        req->res.body = Json::parse(req->buffer);

        if (req->res.status >= 400 && req->res.body.is_object() && req->res.body.contains("detail")) {
            if (const auto& detail = req->res.body.at("detail"); detail.is_string()) {
                req->res.error = detail.get<std::string>();
            }
        }

        finalize(req);
    } catch (std::exception& e) {
        if (req->res.status >= 400 && !req->buffer.empty() && req->buffer.front() != '<') {
            req->res.error = req->buffer;
        } else if (req->res.status == 200 || req->res.status == 201) {
            req->res.error = e.what();
        } else {
            req->res.error = "Server error";
        }
        finalize(req);
    }
}

void NetworkHttpsClient::finalize(const NetworkHttpsClient::HttpRequestPtr& req) {
    if (req->res.error.empty() && req->res.status >= 200 && req->res.status <= 204) {
        logger.info("HTTPS client {} {}{} HTTP/1.1 {}", methodToStr(req->method), url, req->path, req->res.status);
    } else {
        logger.warn("HTTPS client {} {}{} HTTP/1.1 {} {}",
                    methodToStr(req->method),
                    url,
                    req->path,
                    req->res.status,
                    req->res.error);
    }
    req->callback(req->res);
}
