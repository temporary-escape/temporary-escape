#pragma once

#include "../stream/msgpack_acceptor.hpp"
#include "../stream/msgpack_stream.hpp"
#include "network_dispatcher.hpp"

namespace Engine {
class ENGINE_API NetworkPeer : public MsgpackAcceptor, public MsgpackStream {
public:
    virtual ~NetworkPeer() = default;

    virtual bool isConnected() const = 0;
    virtual const std::string& getAddress() const = 0;

    template <typename T> void send(const T& msg, const uint64_t xid) {
        Detail::packMessage(*this, msg, xid);
    }

    template <typename T> void send(const T& msg) {
        const auto xid = nextId.fetch_add(1ULL);
        Detail::packMessage(*this, msg, xid);
    }

private:
    std::atomic_uint64_t nextId{0};
};

template <typename T> inline void BaseRequest::respond(const T& msg) {
    peer->send(msg, xid);
}

inline void BaseRequest::respondError(const std::string& msg) {
    peer->send(msg, xid);
}
} // namespace Engine
