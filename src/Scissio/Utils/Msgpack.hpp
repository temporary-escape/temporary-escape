#pragma once

#include "../Math/Matrix.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Vector.hpp"
#include <msgpack.hpp>
#include <optional>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Scissio::Vector2> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Vector2& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Vector2> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Vector2 const& v) const {
            o.pack_array(2);
            o.pack_float(v.x);
            o.pack_float(v.y);
            return o;
        }
    };

    template <> struct convert<Scissio::Vector2i> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Vector2i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Vector2i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Vector2i const& v) const {
            o.pack_array(2);
            o.pack_int(v.x);
            o.pack_int(v.y);
            return o;
        }
    };

    template <> struct convert<Scissio::Vector3> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Vector3& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Vector3> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Vector3 const& v) const {
            o.pack_array(3);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            return o;
        }
    };

    template <> struct convert<Scissio::Vector3i> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Vector3i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>(), o.via.array.ptr[2].as<int>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Vector3i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Vector3i const& v) const {
            o.pack_array(3);
            o.pack_int(v.x);
            o.pack_int(v.y);
            o.pack_int(v.z);
            return o;
        }
    };

    template <> struct convert<Scissio::Vector4> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Vector4& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>(),
                 o.via.array.ptr[3].as<float>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Vector4> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Vector4 const& v) const {
            o.pack_array(4);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            o.pack_float(v.w);
            return o;
        }
    };

    template <> struct convert<Scissio::Vector4i> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Vector4i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>(), o.via.array.ptr[2].as<int>(),
                 o.via.array.ptr[3].as<int>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Vector4i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Vector4i const& v) const {
            o.pack_array(4);
            o.pack_int(v.x);
            o.pack_int(v.y);
            o.pack_int(v.z);
            o.pack_int(v.w);
            return o;
        }
    };

    template <> struct convert<Scissio::Quaternion> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Quaternion& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>(),
                 o.via.array.ptr[3].as<float>()};
            return o;
        }
    };
    template <> struct pack<Scissio::Quaternion> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Quaternion const& v) const {
            o.pack_array(4);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            o.pack_float(v.w);
            return o;
        }
    };

    template <> struct convert<Scissio::Matrix4> {
        msgpack::object const& operator()(msgpack::object const& o, Scissio::Matrix4& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 16)
                throw msgpack::type_error();
            // v = { o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>(),
            //     o.via.array.ptr[3].as<float>() };
            auto dst = const_cast<float*>(&v[0].x);
            for (auto i = 0; i < 16; i++) {
                dst[i] = o.via.array.ptr[i].as<float>();
            }
            return o;
        }
    };
    template <> struct pack<Scissio::Matrix4> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Scissio::Matrix4 const& v) const {
            o.pack_array(16);
            const auto src = &v[0].x;
            for (auto i = 0; i < 16; i++) {
                o.pack_float(src[i]);
            }
            return o;
            ;
        }
    };

    template <typename T> struct convert<std::optional<T>> {
        msgpack::object const& operator()(msgpack::object const& o, std::optional<T>& v) const {
            if (o.type == msgpack::type::NIL) {
                v = std::nullopt;
                return o;
            }

            v = {o.as<T>()};
            return o;
        }
    };
    template <typename T> struct pack<std::optional<T>> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, std::optional<T> const& v) const {
            if (!v.has_value()) {
                o.pack_nil();
                return o;
            }
            o.pack(v.value());
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack

#define MSGPACK_CONVERT(T)                                                                                             \
    namespace msgpack {                                                                                                \
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {                                                            \
        namespace adaptor {                                                                                            \
        template <> struct convert<T> { msgpack::object const& operator()(msgpack::object const& o, T& v) const; };    \
        template <> struct pack<T> {                                                                                   \
            packer<msgpack::sbuffer>& operator()(msgpack::packer<msgpack::sbuffer>& o, T const& v) const;              \
        };                                                                                                             \
        }                                                                                                              \
    }                                                                                                                  \
    }

#define MSGPACK_UNPACK_FUNC(T)                                                                                         \
    msgpack::object const& msgpack::adaptor::convert<T, void>::operator()(msgpack::object const& o, T& v) const

#define MSGPACK_PACK_FUNC(T)                                                                                           \
    msgpack::packer<msgpack::sbuffer>& msgpack::adaptor::pack<T, void>::operator()(                                    \
        msgpack::packer<msgpack::sbuffer>& o, T const& v) const

#define MSGPACK_FRIEND(T)                                                                                              \
    friend struct msgpack::MSGPACK_DEFAULT_API_NS::adaptor::convert<T>;                                                \
    friend struct msgpack::MSGPACK_DEFAULT_API_NS::adaptor::pack<T>;