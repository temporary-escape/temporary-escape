#pragma once

#include "NetworkTcpConnector.hpp"
#include "NetworkTcpStream.hpp"
#include <any>

namespace Engine {
struct Message {
    uint64_t id{0};
    std::string token;

    MSGPACK_DEFINE_ARRAY(id, token);
};

using NoMessage = void;

#define MESSAGE_APPEND_DEFAULT()                                                                                       \
    Response& operator<<(Response& other) {                                                                            \
        *this = std::move(other);                                                                                      \
        return *this;                                                                                                  \
    }

#define MESSAGE_APPEND_ARRAY(field)                                                                                    \
    Response& operator<<(Response& other) {                                                                            \
        field.insert(field.end(), other.field.begin(), other.field.end());                                             \
        return *this;                                                                                                  \
    }

#define MESSAGE_COPY_DEFAULT()                                                                                         \
    void copyback(Request& req) {}

#define MESSAGE_COPY_FIELDS_1(f0)                                                                                      \
    void copyback(Request& req) { req.f0 = f0; }

template <typename... Types> struct NetworkMessageIndexer;

template <> struct NetworkMessageIndexer<> {
    template <typename M> static size_t indexOf(const size_t idx = 1) {
        return 0;
    }
};

template <typename Type, typename... Types> struct NetworkMessageIndexer<Type, Types...> {
    template <typename M> static size_t indexOf(const size_t idx = 1) {
        if (std::is_same<M, typename Type::Request>::value || std::is_same<M, typename Type::Response>::value) {
            return idx;
        }
        return NetworkMessageIndexer<Types...>::template indexOf<M>(idx + 1);
    }
};

struct AbstractRequestData {
    virtual void handle() = 0;
};

template <typename Response> struct RequestData : AbstractRequestData {
    explicit RequestData(std::function<void(Response)> callback) : callback(std::move(callback)) {
    }

    void handle() override {
        callback(std::move(value));
    }

    std::function<void(Response)> callback;
    Response value;
};

using NetworkRequestsMap = std::unordered_map<uint64_t, std::shared_ptr<AbstractRequestData>>;

template <typename... Types> struct NetworkMessageDispatcherClient;

template <> struct NetworkMessageDispatcherClient<> {
    template <typename Handler, typename Connector>
    static void dispatch(asio::io_service& worker, Handler& handler, NetworkRequestsMap& requests,
                         const std::shared_ptr<Connector>& connector, Packet packet, const size_t idx = 0) {
        EXCEPTION("Unknown packet of id: {}", idx);
    }
};

template <typename Type, typename Request, typename Response> struct NetworkMessageDispatcherClientHelper;

template <typename Type, typename Request, typename Response> struct NetworkMessageDispatcherClientHelper {
    template <typename Handler, typename Connector>
    static void dispatch(asio::io_service& worker, Handler& handler, NetworkRequestsMap& requests,
                         const std::shared_ptr<Connector>& connector, Packet packet) {
        auto res = unpack<Response>(packet.data);

        // We have continuation token
        if (!res.token.empty()) {
            auto it = requests.find(res.id);

            // Either update the existing entry or create a new one
            if (it != requests.end()) {
                auto data = std::dynamic_pointer_cast<RequestData<Response>>(it->second);
                if (!data) {
                    EXCEPTION("Response data type mismatch");
                }

                data->value << res;

                // Send the request again to fetch more entries
                Request req{};
                req.id = res.id;
                req.token = std::move(res.token);
                Log::d("NetworkMessage", "Sending again for token: {}", req.token);
                res.copyback(req);

                connector->send(req);
            }
        }
        // No continuation token. We either have a single
        // message with all data, or this is the last message of the response
        else {
            // Update the entry if it exists
            auto it = requests.find(res.id);

            if (it != requests.end()) {
                auto data = std::dynamic_pointer_cast<RequestData<Response>>(it->second);
                if (!data) {
                    EXCEPTION("Response data type mismatch");
                }

                data->value << res;

                requests.erase(it);

                // Send to client
                worker.post([&handler, data] {
                    try {
                        data->handle();
                    } catch (std::exception& e) {
                        BACKTRACE("NetworkMessageDispatcherClient", e, "Error handling message of type '{}'",
                                  typeid(Type).name());
                    }
                });
            }
        }
    }
};

template <typename Type, typename Response> struct NetworkMessageDispatcherClientHelper<Type, void, Response> {
    template <typename Handler, typename Connector>
    static void dispatch(asio::io_service& worker, Handler& handler, NetworkRequestsMap& requests,
                         const std::shared_ptr<Connector>& connector, Packet packet) {
        auto res = unpack<Response>(packet.data);

        // Send to client
        worker.post([&handler, res = std::move(res)] {
            try {
                handler.handle(std::move(res));
            } catch (std::exception& e) {
                BACKTRACE("NetworkMessageDispatcherClient", e, "Error handling message of type '{}'",
                          typeid(Type).name());
            }
        });
    }
};

template <typename Type, typename... Types> struct NetworkMessageDispatcherClient<Type, Types...> {
    template <typename Handler, typename Connector>
    static void dispatch(asio::io_service& worker, Handler& handler, NetworkRequestsMap& requests,
                         const std::shared_ptr<Connector>& connector, Packet packet, const size_t idx = 1) {
        if (idx == packet.id) {
            using Helper = NetworkMessageDispatcherClientHelper<Type, typename Type::Request, typename Type::Response>;
            Helper::dispatch(worker, handler, requests, connector, std::move(packet));
        } else {
            NetworkMessageDispatcherClient<Types...>::template dispatch(worker, handler, requests, connector,
                                                                        std::move(packet), idx + 1);
        }
    }
};

template <typename... Types> class NetworkMessageSinkClient {
public:
    template <typename F, typename Ret, typename A, typename... Rest> static A helper(Ret (F::*)(A, Rest...));

    template <typename F, typename Ret, typename A, typename... Rest> static A helper(Ret (F::*)(A, Rest...) const);

    template <typename F> struct FirstArgument { typedef decltype(helper(&F::operator())) type; };

    virtual ~NetworkMessageSinkClient() = default;

    template <typename Handler, typename Connector>
    void dispatch(asio::io_service& worker, Handler& handler, const std::shared_ptr<Connector>& connector,
                  Packet packet) {
        NetworkMessageDispatcherClient<Types...>::dispatch(worker, handler, requests, connector, std::move(packet));
    }

    template <typename Fn> void allocateRequestData(Fn&& callback, uint64_t& id) {
        using R = typename FirstArgument<Fn>::type;

        id = requestId.fetch_add(1);

        auto data = std::make_shared<RequestData<R>>(std::forward<Fn>(callback));
        // data.callback = [callback](std::any& value) { callback(std::any_cast<R>(value)); };
        requests.insert(std::make_pair(id, std::move(data)));
    }

private:
    std::atomic<uint64_t> requestId{1};
    NetworkRequestsMap requests;
};

template <typename... Types> struct NetworkMessageDispatcherServer;

template <> struct NetworkMessageDispatcherServer<> {
    template <typename Handler, typename Peer>
    static void dispatch(asio::io_service& worker, Handler& handler, const std::shared_ptr<Peer>& peer, Packet packet,
                         const size_t idx = 0) {
        EXCEPTION("Unknown packet of id: {}", idx);
    }
};

template <typename Type, typename Request, typename Response> struct NetworkMessageDispatcherServerHelper;

template <typename Type, typename Request, typename Response> struct NetworkMessageDispatcherServerHelper {
    template <typename Handler, typename Peer>
    static void dispatch(asio::io_service& worker, Handler& handler, const std::shared_ptr<Peer>& peer, Packet packet) {
        auto req = unpack<Request>(packet.data);
        worker.post([&handler, peer = peer, req = std::move(req)] {
            try {
                Response res{};
                res.id = req.id;
                handler.handle(peer, std::move(req), res);
                peer->send(res);
            } catch (std::exception& e) {
                BACKTRACE("NetworkMessageDispatcherServer", e, "Error handling message of type '{}'",
                          typeid(Type).name());
            }
        });
    }
};

template <typename Type, typename Response> struct NetworkMessageDispatcherServerHelper<Type, void, Response> {
    template <typename Handler, typename Peer>
    static void dispatch(asio::io_service& worker, Handler& handler, const std::shared_ptr<Peer>& peer, Packet packet) {
        (void)packet;
    }
};

template <typename Type, typename... Types> struct NetworkMessageDispatcherServer<Type, Types...> {
    template <typename Handler, typename Peer>
    static void dispatch(asio::io_service& worker, Handler& handler, const std::shared_ptr<Peer>& peer, Packet packet,
                         const size_t idx = 1) {
        using Helper = NetworkMessageDispatcherServerHelper<Type, typename Type::Request, typename Type::Response>;

        if (idx == packet.id) {
            Helper::dispatch(worker, handler, peer, std::move(packet));
        } else {
            NetworkMessageDispatcherServer<Types...>::template dispatch<Handler, Peer>(worker, handler, peer,
                                                                                       std::move(packet), idx + 1);
        }
    }
};

template <typename... Types> class NetworkMessageSinkServer {
public:
    virtual ~NetworkMessageSinkServer() = default;

    template <typename Handler, typename Peer>
    void dispatch(asio::io_service& worker, Handler& handler, const std::shared_ptr<Peer>& peer, Packet packet) {
        NetworkMessageDispatcherServer<Types...>::template dispatch(worker, handler, peer, std::move(packet));
    }
};

template <typename... Types> class NetworkMessageSink {
public:
    using Server = NetworkMessageSinkServer<Types...>;
    using Client = NetworkMessageSinkClient<Types...>;

    template <typename M> static size_t indexOf() {
        return NetworkMessageIndexer<Types...>::template indexOf<M>();
    }
};
} // namespace Engine
