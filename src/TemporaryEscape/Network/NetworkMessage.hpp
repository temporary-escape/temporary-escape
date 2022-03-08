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

struct NoMessage : Message {
    MSGPACK_DEFINE_ARRAY();
};

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
    void copyback(Request& req) {                                                                                      \
    }

#define MESSAGE_COPY_FIELDS_1(f0)                                                                                      \
    void copyback(Request& req) {                                                                                      \
        req.f0 = f0;                                                                                                   \
    }

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

template <typename... Types> class NetworkMessageHandlerClient;

template <> class NetworkMessageHandlerClient<> {};

template <typename Type, typename... Types>
class NetworkMessageHandlerClient<Type, Types...> : public NetworkMessageHandlerClient<Types...> {
public:
    virtual void handle(typename Type::Response res) = 0;
};

using NetworkRequestsMap = std::unordered_map<uint64_t, std::any>;

template <typename... Types> struct NetworkMessageDispatcherClient;

template <> struct NetworkMessageDispatcherClient<> {
    template <typename Connector>
    static void dispatch(asio::io_service& worker, NetworkMessageHandlerClient<>& handler, NetworkRequestsMap& requests,
                         const std::shared_ptr<Connector>& connector, Packet packet, const size_t idx = 0) {
        EXCEPTION("Unknown packet of id: {}", idx);
    }
};

template <typename Type, typename... Types> struct NetworkMessageDispatcherClient<Type, Types...> {
    template <typename Connector>
    static void dispatch(asio::io_service& worker, NetworkMessageHandlerClient<Type, Types...>& handler,
                         NetworkRequestsMap& requests, const std::shared_ptr<Connector>& connector, Packet packet,
                         const size_t idx = 1) {
        if (idx == packet.id) {
            auto res = unpack<typename Type::Response>(packet.data);

            // We have continuation token
            if (!res.token.empty()) {
                auto it = requests.find(res.id);

                // Send the request again to fetch more entries
                typename Type::Request req{};
                req.id = res.id;
                req.token = std::move(res.token);
                res.copyback(req);

                // Either update the existing entry or create a new one
                if (it != requests.end()) {
                    auto& previous = std::any_cast<typename Type::Response&>(it->second);
                    previous << res;
                } else {
                    requests.insert(std::make_pair(res.id, std::move(res)));
                }

                connector->send(req);
            }
            // No continuation token. We either have a single
            // message with all data, or this is the last message of the response
            else {
                // Update the entry if it exists
                auto it = requests.find(res.id);
                if (it != requests.end()) {
                    auto& previous = std::any_cast<typename Type::Response&>(it->second);
                    previous << res;
                    std::swap(res, previous);
                    requests.erase(it);
                }

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
        } else {
            NetworkMessageDispatcherClient<Types...>::template dispatch(worker, handler, requests, connector,
                                                                        std::move(packet), idx + 1);
        }
    }
};

template <typename Connector, typename... Types>
class NetworkMessageSinkClient : public NetworkMessageHandlerClient<Types...> {
public:
    virtual ~NetworkMessageSinkClient() = default;

    void dispatch(asio::io_service& worker, const std::shared_ptr<Connector>& connector, Packet packet) {
        NetworkMessageDispatcherClient<Types...>::dispatch(
            worker, static_cast<NetworkMessageHandlerClient<Types...>&>(*this), requests, connector, std::move(packet));
    }

    uint64_t nextRequestId() {
        return requestId.fetch_add(1);
    }

private:
    std::atomic<uint64_t> requestId{1};
    NetworkRequestsMap requests;
};

template <typename Peer, typename... Types> class NetworkMessageHandlerServer;

template <typename Peer> class NetworkMessageHandlerServer<Peer> {};

template <typename Peer, typename Type, typename... Types>
class NetworkMessageHandlerServer<Peer, Type, Types...> : public NetworkMessageHandlerServer<Peer, Types...> {
public:
    virtual void handle(const std::shared_ptr<Peer>& peer, typename Type::Request req,
                        typename Type::Response& res) = 0;
};

template <typename... Types> struct NetworkMessageDispatcherServer;

template <> struct NetworkMessageDispatcherServer<> {
    template <typename Peer>
    static void dispatch(asio::io_service& worker, NetworkMessageHandlerServer<Peer>& handler,
                         const std::shared_ptr<Peer>& peer, Packet packet, const size_t idx = 0) {
        EXCEPTION("Unknown packet of id: {}", idx);
    }
};

template <typename Type, typename... Types> struct NetworkMessageDispatcherServer<Type, Types...> {
    template <typename Peer>
    static void dispatch(asio::io_service& worker, NetworkMessageHandlerServer<Peer, Type, Types...>& handler,
                         const std::shared_ptr<Peer>& peer, Packet packet, const size_t idx = 1) {
        if (idx == packet.id) {
            auto req = unpack<typename Type::Request>(packet.data);
            worker.post([&handler, peer = peer, req = std::move(req)] {
                try {
                    typename Type::Response res{};
                    res.id = req.id;
                    handler.handle(peer, std::move(req), res);
                    peer->send(res);
                } catch (std::exception& e) {
                    BACKTRACE("NetworkMessageDispatcherServer", e, "Error handling message of type '{}'",
                              typeid(Type).name());
                }
            });
        } else {
            NetworkMessageDispatcherServer<Types...>::template dispatch<Peer>(worker, handler, peer, std::move(packet),
                                                                              idx + 1);
        }
    }
};

template <typename Peer, typename... Types>
class NetworkMessageSinkServer : public NetworkMessageHandlerServer<Peer, Types...> {
public:
    virtual ~NetworkMessageSinkServer() = default;

    void dispatch(asio::io_service& worker, const std::shared_ptr<Peer>& peer, Packet packet) {
        NetworkMessageDispatcherServer<Types...>::template dispatch(
            worker, static_cast<NetworkMessageHandlerServer<Peer, Types...>&>(*this), peer, std::move(packet));
    }
};

template <typename... Types> class NetworkMessageSink {
public:
    template <typename Peer> using Server = NetworkMessageSinkServer<Peer, Types...>;
    template <typename Connector> using Client = NetworkMessageSinkClient<Connector, Types...>;

    template <typename M> static size_t indexOf() {
        return NetworkMessageIndexer<Types...>::template indexOf<M>();
    }
};
} // namespace Engine
