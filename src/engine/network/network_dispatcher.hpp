#pragma once

#include "network_message.hpp"
#include <functional>
#include <memory>

namespace Engine {
class ENGINE_API NetworkTcpPeer;
class ENGINE_API NetworkDispatcher {
public:
    using ObjectHandlePtr = std::shared_ptr<msgpack::object_handle>;
    using Handler = std::function<void(NetworkPeerPtr, ObjectHandlePtr)>;

    virtual ~NetworkDispatcher() = default;

    template <typename C, typename T> void addHandlerC(C* instance, void (C::*fn)(Request<T>)) {
        HandlerData data;
        data.fn = [instance, fn](NetworkPeerPtr peer, ObjectHandlePtr oh) {
            // Construct a request and call a handler function
            instance->*fn(Request<T>{std::move(peer), std::move(oh)});
        };
        data.name = Detail::MessageHelper<T>::name;

        handlers.emplace(Detail::MessageHelper<T>::hash, std::move(data));
    }

    template <typename Fn> void addHandler(Fn&& fn) {
        using Req = typename Traits<decltype(&Fn::operator())>::Arg;
        using T = Req;

        HandlerData data;
        data.fn = [fn](NetworkPeerPtr peer, ObjectHandlePtr oh) {
            // Construct a request and call a handler function
            fn(Request<T>{std::move(peer), std::move(oh)});
        };
        data.name = Detail::MessageHelper<T>::name;

        handlers.emplace(Detail::MessageHelper<T>::hash, std::move(data));
    }

    void onObjectReceived(NetworkPeerPtr peer, ObjectHandlePtr oh);

    virtual void onAcceptSuccess(const NetworkPeerPtr& peer) {
        (void)peer;
    };
    virtual void onDisconnect(const NetworkPeerPtr& peer) {
        (void)peer;
    };

private:
    struct HandlerData {
        Handler fn;
        const char* name;
    };

    template <typename F> struct Traits;

    template <typename C, typename R, typename T> struct Traits<R (C::*)(Request<T>) const> {
        using Ret = R;
        using Arg = T;
    };

    std::unordered_map<uint64_t, HandlerData> handlers;
};
} // namespace Engine

#define HANDLE_REQUEST(Type)                                                                                           \
    addHandler([this](Request<Type> req) {                                                                             \
        using Self = std::remove_pointer<decltype(this)>::type;                                                        \
        using Handler = void (Self::*)(Request<Type>);                                                                 \
        (this->*static_cast<Handler>(&Self::handle))(std::move(req));                                                  \
    })
