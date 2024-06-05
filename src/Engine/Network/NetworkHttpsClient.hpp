#include "../Future.hpp"
#include "../Utils/Json.hpp"
#include "NetworkUtils.hpp"
#include <vector>

namespace Engine {
struct HttpResponse {
    int status{0};
    std::string error;
    Json body;
    std::unordered_map<std::string, std::string> headers;

    std::string formatError() const;
};

enum class HttpMethod {
    Get,
    Post,
    Put,
    Delete,
};

using HttpResponseCallback = std::function<void(const HttpResponse&)>;

class ENGINE_API NetworkHttpsClient : public std::enable_shared_from_this<NetworkHttpsClient> {
public:
    NetworkHttpsClient(asio::io_context& service, std::string url);
    virtual ~NetworkHttpsClient();

    void request(HttpMethod method, std::string path, Json json, HttpResponseCallback callback);
    void stop();
    const std::string& getAuthorization() const {
        return token;
    }
    void setAuthorization(const std::string& value);

private:
    using Socket = asio::ssl::stream<asio::ip::tcp::socket>;

    struct HttpRequest {
        HttpRequest(asio::io_context& service, asio::ssl::context& ssl) : socket{service, ssl} {
        }

        Socket socket;
        HttpMethod method;
        std::string path;
        HttpResponseCallback callback;
        std::string buffer;
        Json json;
        HttpResponse res;
        size_t contentLength{0};
        size_t consumed{0};
    };

    using HttpRequestPtr = std::shared_ptr<HttpRequest>;

    void resolve(const HttpRequestPtr& req);
    void connect(const HttpRequestPtr& req);
    void handshake(const HttpRequestPtr& req);
    void send(const HttpRequestPtr& req);
    void read(const HttpRequestPtr& req);
    void processBody(const HttpRequestPtr& req);
    void processCookies(const std::string_view& header);
    void errored(const HttpRequestPtr& req, const std::string_view& msg);
    void finalize(const HttpRequestPtr& req);

    asio::io_context& service;
    asio::io_context::strand strand;
    std::string url;
    std::string host;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), 0};
    std::unique_ptr<asio::ip::tcp::resolver::query> query;
    asio::ssl::context ssl;
    std::string token;
};
} // namespace Engine
