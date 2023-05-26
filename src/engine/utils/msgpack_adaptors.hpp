#pragma once

#include <chrono>
#include <msgpack/adaptor/define_decl.hpp>
#include <optional>
#include <variant>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<std::chrono::steady_clock::time_point> {
        msgpack::object const& operator()(msgpack::object const& o, std::chrono::steady_clock::time_point& v) const {
            if (o.type != msgpack::type::POSITIVE_INTEGER) {
                throw msgpack::type_error();
            }

            uint64_t timeSinceEpoch{0};
            o.convert(timeSinceEpoch);

            v = std::chrono::steady_clock::time_point{std::chrono::milliseconds{timeSinceEpoch}};
            return o;
        }
    };
    template <> struct pack<std::chrono::steady_clock::time_point> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, std::chrono::steady_clock::time_point const& v) const {
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(v.time_since_epoch());
            o.pack_int64(ms.count());
            return o;
        }
    };

    template <> struct convert<std::chrono::system_clock::time_point> {
        msgpack::object const& operator()(msgpack::object const& o, std::chrono::system_clock::time_point& v) const {
            if (o.type != msgpack::type::POSITIVE_INTEGER) {
                throw msgpack::type_error();
            }

            uint64_t timeSinceEpoch{0};
            o.convert(timeSinceEpoch);

            v = std::chrono::system_clock::time_point{std::chrono::milliseconds{timeSinceEpoch}};
            return o;
        }
    };
    template <> struct pack<std::chrono::system_clock::time_point> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, std::chrono::system_clock::time_point const& v) const {
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(v.time_since_epoch());
            o.pack_int64(ms.count());
            return o;
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

    template <typename... T> struct variant_adaptor;

    template <> struct variant_adaptor<> {
        template <typename Variant> static void convert(msgpack::object const& o, Variant& v, const size_t idx = 0) {
            (void)o;
            (void)v;
            (void)idx;
        }

        template <typename Stream, typename Variant>
        static void pack(msgpack::packer<Stream>& o, Variant const& v, const size_t idx = 0) {
            (void)o;
            (void)v;
        }
    };

    template <typename T, typename... Other> struct variant_adaptor<T, Other...> {
        template <typename Variant> static void convert(msgpack::object const& o, Variant& v, const size_t idx = 0) {
            if (o.type != msgpack::type::ARRAY) {
                throw msgpack::type_error();
            }

            if (o.via.array.size != 2) {
                throw msgpack::type_error();
            }

            if (o.via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) {
                throw msgpack::type_error();
            }

            if (o.via.array.ptr[0].via.i64 == idx) {
                v = T{};
                auto& vv = std::get<T>(v);
                o.via.array.ptr[1].convert(vv);
            } else {
                variant_adaptor<Other...>::template convert(o, v, idx + 1);
            }
        }

        template <typename Stream, typename Variant>
        static void pack(msgpack::packer<Stream>& o, Variant const& v, const size_t idx = 0) {
            if (v.index() == idx) {
                const auto& vv = std::get<T>(v);
                o.pack_array(2);
                o.pack(idx);
                o.pack(vv);
            } else {
                variant_adaptor<Other...>::template pack(o, v, idx + 1);
            }
        }
    };

    template <typename... T> struct convert<std::variant<T...>> {
        msgpack::object const& operator()(msgpack::object const& o, std::variant<T...>& v) const {
            variant_adaptor<T...>::template convert(o, v);
            return o;
        }
    };

    template <typename... T> struct pack<std::variant<T...>> {
        template <typename Stream>
        packer<Stream>& operator()(msgpack::packer<Stream>& o, std::variant<T...> const& v) const {
            variant_adaptor<T...>::template pack(o, v);
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
