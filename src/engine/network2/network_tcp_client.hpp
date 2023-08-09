#pragma once

#include "../future.hpp"
#include "../utils/exceptions.hpp"
#include "network_ssl_context.hpp"
#include "network_tcp_peer.hpp"
#include <asio.hpp>

namespace Engine {
class ENGINE_API NetworkTcpClient {
public:
    explicit NetworkTcpClient(asio::io_service& service, NetworkDispatcher& dispatcher, const std::string& host,
                              uint32_t port);
    virtual ~NetworkTcpClient();

    void close();

    bool isConnected() const {
        return internal && internal->isConnected();
    }
    const std::string& getAddress() {
        return internal->getAddress();
    }

    template <typename T> void send(const T& msg) {
        internal->send(msg, 0);
    }

    template <typename Res, typename Req> inline PromisePtr<Res> request(const Req& req) {
        if (!internal) {
            EXCEPTION("Client is not connected, can not create a request");
        }
        return internal->request<Res, Req>(req);
    }

private:
    class Internal : public NetworkPeer, public std::enable_shared_from_this<Internal> {
    public:
        Internal(asio::io_service& service, NetworkDispatcher& dispatcher);
        void close();
        void connect(const std::string& host, uint32_t port, std::chrono::milliseconds timeout);
        void receive();
        bool isConnected() const override;
        const std::string& getAddress() const override {
            return address;
        }

        template <typename Res, typename Req> inline PromisePtr<Res> request(const Req& req) {
            auto self = shared_from_this();
            auto promise = std::make_shared<Promise<Res>>();
            const auto xid = getNextRequestSlot();
            auto& handler = requests.map.at(xid);

            handler.fn = [self, promise](ObjectHandlePtr oh) {
                Request<Res> res{self, std::move(oh)};
                try {
                    promise->resolve(std::move(res.get()));
                } catch (...) {
                    auto eptr = std::current_exception();
                    promise->reject(eptr);
                }
            };

            send(req, xid + 1);
            return promise;
        }

    protected:
        void receiveObject(msgpack::object_handle oh) override;
        void writeCompressed(const char* data, size_t length) override;

    private:
        struct RequestHandler {
            bool used{false};
            std::function<void(ObjectHandlePtr)> fn;
        };

        uint64_t getNextRequestSlot();
        void handleRequestSlot(uint64_t value, ObjectHandlePtr oh);
        void resetRequestSlot(uint64_t value);

        asio::io_service& service;
        NetworkDispatcher& dispatcher;
        asio::io_service::strand strand;
        Socket socket;
        std::string address;
        std::array<char, 4096> buffer{};

        struct {
            std::mutex mutex;
            std::array<RequestHandler, 128> map;
        } requests;
    };

    std::shared_ptr<Internal> internal;
};
} // namespace Engine
