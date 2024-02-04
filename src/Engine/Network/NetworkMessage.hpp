#pragma once

#include "../Stream/MsgpackStream.hpp"
#include <fmt/format.h>
#include <typeindex>
#include <unordered_map>

namespace Engine {
class ENGINE_API NetworkPeer;
class ENGINE_API NetworkStream;

using NetworkPeerPtr = std::shared_ptr<NetworkPeer>;
using NetworkStreamPtr = std::shared_ptr<NetworkStream>;
using ObjectHandlePtr = std::shared_ptr<msgpack::object_handle>;

template <typename T> struct UseFuture {
    using Type = T;
};

namespace Detail {
ENGINE_API uint64_t getMessageHash(const std::string_view& name);

template <typename T> struct MessageHelper {};

template <typename T> inline void prepareMessage(MsgpackStream& stream, const uint64_t xid) {
    stream.pack_array(3);
    stream.pack_uint64(MessageHelper<T>::hash);
    stream.pack_uint64(xid);
}

template <typename T> inline void packMessage(MsgpackStream& stream, const T& msg, const uint64_t xid) {
    stream.pack_array(3);
    stream.pack_uint64(MessageHelper<T>::hash);
    stream.pack_uint64(xid);
    stream.pack(msg);
    stream.flush();
}

template <> inline void packMessage<std::string>(MsgpackStream& stream, const std::string& msg, const uint64_t xid) {
    stream.pack_array(3);
    stream.pack_uint64(0);
    stream.pack_uint64(xid);
    stream.pack(msg);
    stream.flush();
}

ENGINE_API bool validateMessageObject(const msgpack::object_handle& oh);
} // namespace Detail

class ENGINE_API BaseRequest {
public:
    explicit BaseRequest(NetworkPeerPtr peer, ObjectHandlePtr oh) : peer{std::move(peer)}, oh{std::move(oh)} {
        const auto& o = this->oh->get();
        o.via.array.ptr[1].convert(xid);
    }
    virtual ~BaseRequest() = default;
    COPYABLE(BaseRequest);
    MOVEABLE(BaseRequest);

    template <typename T> void respond(const T& msg) const;

    void respondError(const std::string& msg) const;

    template <typename... Args> void respondError(const std::string& msg, Args&&... args) const {
        respondError(fmt::format(msg, std::forward<Args>(args)...));
    }

    [[nodiscard]] bool isError() const {
        return object().type == msgpack::type::STR;
    }

    [[nodiscard]] std::string getError() const {
        return object().as<std::string>();
    }

    [[nodiscard]] const msgpack::object& object() const {
        const auto& o = this->oh->get();
        return o.via.array.ptr[2];
    }

    NetworkPeerPtr peer;

protected:
    ObjectHandlePtr oh;
    uint64_t xid;
};

template <typename T> class Request : public BaseRequest {
public:
    using Type = T;

    explicit Request(NetworkPeerPtr peer, ObjectHandlePtr oh) : BaseRequest{std::move(peer), std::move(oh)} {
    }

    [[nodiscard]] T get() const {
        if (isError()) {
            throw std::runtime_error(object().template as<std::string>());
        }

        return object().template as<T>();
    }

    COPYABLE(Request);
    MOVEABLE(Request);
};

class ENGINE_API BaseRequest2 {
public:
    explicit BaseRequest2(NetworkStreamPtr peer, ObjectHandlePtr oh) : peer{std::move(peer)}, oh{std::move(oh)} {
        const auto& o = this->oh->get();
        o.via.array.ptr[1].convert(xid);
    }
    virtual ~BaseRequest2() = default;
    COPYABLE(BaseRequest2);
    MOVEABLE(BaseRequest2);

    template <typename T> void respond(const T& msg) const;

    void respondError(const std::string& msg) const;

    template <typename... Args> void respondError(const std::string& msg, Args&&... args) const {
        respondError(fmt::format(msg, std::forward<Args>(args)...));
    }

    [[nodiscard]] bool isError() const {
        return object().type == msgpack::type::STR;
    }

    [[nodiscard]] std::string getError() const {
        return object().as<std::string>();
    }

    [[nodiscard]] const msgpack::object& object() const {
        const auto& o = this->oh->get();
        return o.via.array.ptr[2];
    }

    NetworkStreamPtr peer;

protected:
    ObjectHandlePtr oh;
    uint64_t xid;
};

template <typename T> class Request2 : public BaseRequest2 {
public:
    using Type = T;

    explicit Request2(NetworkStreamPtr peer, ObjectHandlePtr oh) : BaseRequest2{std::move(peer), std::move(oh)} {
    }

    [[nodiscard]] T get() const {
        if (isError()) {
            throw std::runtime_error(object().template as<std::string>());
        }

        return object().template as<T>();
    }

    COPYABLE(Request2);
    MOVEABLE(Request2);
};
} // namespace Engine

#define MESSAGE_DEFINE(Type)                                                                                           \
    template <> struct Engine::Detail::MessageHelper<Type> {                                                           \
        static inline const uint64_t hash = Engine::Detail::getMessageHash(#Type);                                     \
        static inline const char* name = #Type;                                                                        \
    }
