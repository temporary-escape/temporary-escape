#pragma once

#include "../stream/msgpack_stream.hpp"
#include <typeindex>
#include <unordered_map>

namespace Engine {
class ENGINE_API NetworkPeer;

using NetworkPeerPtr = std::shared_ptr<NetworkPeer>;
using ObjectHandlePtr = std::shared_ptr<msgpack::object_handle>;

template <typename T> struct UseFuture {
    using Type = T;
};

namespace Detail {
ENGINE_API uint64_t getMessageHash(const std::string_view& name);

template <typename T> struct MessageHelper {};

template <typename T> inline void packMessage(MsgpackStream& stream, const T& msg, uint64_t xid) {
    stream.pack_array(3);
    stream.pack_uint64(Detail::MessageHelper<T>::hash);
    stream.pack_uint64(xid);
    stream.pack(msg);
    stream.flush();
}

template <> inline void packMessage<std::string>(MsgpackStream& stream, const std::string& msg, uint64_t xid) {
    stream.pack_array(3);
    stream.pack_uint64(0);
    stream.pack_uint64(xid);
    stream.pack(msg);
    stream.flush();
}

ENGINE_API bool validateMessageObject(const msgpack::object_handle& oh);
} // namespace Detail

class ENGINE_API NetworkPeer;

class ENGINE_API BaseRequest {
public:
    explicit BaseRequest(NetworkPeerPtr peer, ObjectHandlePtr object) :
        peer{std::move(peer)}, object{std::move(object)} {
        const auto& o = this->object->get();
        o.via.array.ptr[1].convert(xid);
    }
    virtual ~BaseRequest() = default;
    NON_COPYABLE(BaseRequest);
    MOVEABLE(BaseRequest);

    template <typename T> void respond(const T& msg);

    void respondError(const std::string& msg);

    [[nodiscard]] bool isError() const {
        const auto& o = this->object->get();
        return o.via.array.ptr[2].type == msgpack::type::STR;
    }

    NetworkPeerPtr peer;
    ObjectHandlePtr object;
    uint64_t xid;
};

template <typename T> class ENGINE_API Request : public BaseRequest {
public:
    using Type = T;

    explicit Request(NetworkPeerPtr peer, ObjectHandlePtr object) : BaseRequest{std::move(peer), std::move(object)} {
    }

    T get() {
        const auto& o = this->object->get();
        if (isError()) {
            throw std::runtime_error(o.via.array.ptr[2].template as<std::string>());
        }

        return o.via.array.ptr[2].template as<T>();
    }

    NON_COPYABLE(Request);
    MOVEABLE(Request);
};

/*class ENGINE_API RawMessage {
public:
    explicit RawMessage(std::shared_ptr<msgpack::object_handle> oh) : oh{std::move(oh)} {
    }

    [[nodiscard]] const msgpack::object& get() const {
        if (!oh) {
            throw std::bad_cast();
        }
        return oh->get().via.array.ptr[1];
    }

private:
    std::shared_ptr<msgpack::object_handle> oh;
};*/
} // namespace Engine

#define MESSAGE_DEFINE(Type)                                                                                           \
    template <> struct Engine::Detail::MessageHelper<Type> {                                                           \
        static inline const uint64_t hash = Engine::Detail::getMessageHash(#Type);                                     \
        static inline const char* name = #Type;                                                                        \
    }
