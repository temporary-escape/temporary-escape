#pragma once

#include "NetworkMessage.hpp"
#include <functional>
#include <memory>

namespace Engine {
class ENGINE_API NetworkDispatcher2 {
public:
    using ObjectHandlePtr = std::shared_ptr<msgpack::object_handle>;
    using Handler = std::function<void(NetworkStreamPtr, ObjectHandlePtr)>;

    virtual ~NetworkDispatcher2() = default;

    template <typename Fn> void addHandler(Fn&& fn) {
        using Req = typename Traits<decltype(&Fn::operator())>::Arg;
        using T = Req;

        HandlerData data;
        data.fn = [fn](NetworkStreamPtr peer, ObjectHandlePtr oh) {
            // Construct a request and call a handler function
            fn(Request2<T>{std::move(peer), std::move(oh)});
        };
        data.name = Detail::MessageHelper<T>::name;

        handlers.emplace(Detail::MessageHelper<T>::hash, std::move(data));
    }

    void onObjectReceived(NetworkStreamPtr peer, ObjectHandlePtr oh);

    virtual void onAcceptSuccess(const NetworkStreamPtr& peer) {
        (void)peer;
    };
    virtual void onDisconnect(const NetworkStreamPtr& peer) {
        (void)peer;
    };

private:
    struct HandlerData {
        Handler fn;
        const char* name;
    };

    template <typename F> struct Traits;

    template <typename C, typename R, typename T> struct Traits<R (C::*)(Request2<T>) const> {
        using Ret = R;
        using Arg = T;
    };

    std::unordered_map<uint64_t, HandlerData> handlers;
};
} // namespace Engine

#define HANDLE_REQUEST2(Type)                                                                                          \
    dispatcher.addHandler([this](Request2<Type> req) {                                                                 \
        using Self = std::remove_pointer<decltype(this)>::type;                                                        \
        using Handler = void (Self::*)(Request2<Type>);                                                                \
        (this->*static_cast<Handler>(&Self::handle))(std::move(req));                                                  \
    })
