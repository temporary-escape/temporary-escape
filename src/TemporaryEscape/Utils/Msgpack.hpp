#pragma once

#include "../Math/Matrix.hpp"
#include "../Math/Quaternion.hpp"
#include "../Math/Vector.hpp"
#include <msgpack.hpp>
#include <msgpack/adaptor/define_decl.hpp>
#include <optional>
#include <sstream>

namespace Engine {
class MsgpackJsonVisitor : public msgpack::v2::null_visitor {
public:
    explicit MsgpackJsonVisitor(std::ostream& os) : os(os) {
    }

    bool visit_nil() {
        os << "null";
        return true;
    }

    bool visit_boolean(bool v) {
        if (v)
            os << "true";
        else
            os << "false";
        return true;
    }

    bool visit_positive_integer(uint64_t v) {
        os << v;
        return true;
    }

    bool visit_negative_integer(int64_t v) {
        os << v;
        return true;
    }

    bool visit_float32(float v) {
        os << v;
        return true;
    }

    bool visit_float64(double v) {
        os << v;
        return true;
    }

    bool visit_bin(const char* /*v*/, uint32_t size) {
        os << "\"<binary data of size " << size << ">\"";
        return true;
    }

    bool visit_str(const char* v, uint32_t size) {
        os << "\"" << std::string(v, size) << "\"";
        first = true;
        return true;
    }

    bool start_array(uint32_t) {
        os << "[";
        first = true;
        return true;
    }

    bool start_array_item() {
        if (!first) {
            os << ",";
        }
        first = false;
        return true;
    }

    bool end_array() {
        os << "]";
        return true;
    }

    bool start_map(uint32_t) {
        os << "{";
        return true;
    }

    bool start_map_key() {
        if (!first) {
            os << ",";
        }
        first = false;
        return true;
    }

    bool end_map_key() {
        os << ":";
        return true;
    }

    bool end_map() {
        os << "}";
        return true;
    }

private:
    std::ostream& os;
    bool first{true};
};

template <typename Container> inline std::string msgpackToJson(const Container& data) {
    std::stringstream ss;
    MsgpackJsonVisitor visitor(ss);
    std::size_t off = 0;
    msgpack::v2::parse(reinterpret_cast<const char*>(data.data()), data.size(), off, visitor);
    return ss.str();
}
}; // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<Engine::Vector2> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector2& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector2> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector2 const& v) const {
            o.pack_array(2);
            o.pack_float(v.x);
            o.pack_float(v.y);
            return o;
        }
    };

    template <> struct convert<Engine::Vector2i> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector2i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 2)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector2i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector2i const& v) const {
            o.pack_array(2);
            o.pack_int(v.x);
            o.pack_int(v.y);
            return o;
        }
    };

    template <> struct convert<Engine::Vector3> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector3& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector3> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector3 const& v) const {
            o.pack_array(3);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            return o;
        }
    };

    template <> struct convert<Engine::Vector3i> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector3i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>(), o.via.array.ptr[2].as<int>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector3i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector3i const& v) const {
            o.pack_array(3);
            o.pack_int(v.x);
            o.pack_int(v.y);
            o.pack_int(v.z);
            return o;
        }
    };

    template <> struct convert<Engine::Vector4> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector4& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>(),
                 o.via.array.ptr[3].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector4> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector4 const& v) const {
            o.pack_array(4);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            o.pack_float(v.w);
            return o;
        }
    };

    template <> struct convert<Engine::Vector4i> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Vector4i& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<int>(), o.via.array.ptr[1].as<int>(), o.via.array.ptr[2].as<int>(),
                 o.via.array.ptr[3].as<int>()};
            return o;
        }
    };
    template <> struct pack<Engine::Vector4i> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Vector4i const& v) const {
            o.pack_array(4);
            o.pack_int(v.x);
            o.pack_int(v.y);
            o.pack_int(v.z);
            o.pack_int(v.w);
            return o;
        }
    };

    template <> struct convert<Engine::Quaternion> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Quaternion& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 4)
                throw msgpack::type_error();
            v = {o.via.array.ptr[0].as<float>(), o.via.array.ptr[1].as<float>(), o.via.array.ptr[2].as<float>(),
                 o.via.array.ptr[3].as<float>()};
            return o;
        }
    };
    template <> struct pack<Engine::Quaternion> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Quaternion const& v) const {
            o.pack_array(4);
            o.pack_float(v.x);
            o.pack_float(v.y);
            o.pack_float(v.z);
            o.pack_float(v.w);
            return o;
        }
    };

    template <> struct convert<Engine::Matrix4> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::Matrix4& v) const {
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
    template <> struct pack<Engine::Matrix4> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::Matrix4 const& v) const {
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
