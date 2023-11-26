#pragma once

#include <msgpack.hpp>

#include "Exceptions.hpp"
#include <msgpack.hpp>
#include <msgpack/adaptor/define_decl.hpp>
#include <optional>
#include <sstream>
#include <variant>

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

template <typename T, typename Container> T unpack(const Container& buffer) {
    msgpack::object_handle obj;
    msgpack::unpack(obj, reinterpret_cast<const char*>(buffer.data()), buffer.size());
    const auto& o = obj.get();

    T ret{};

    try {
        o.convert(ret);
    } catch (...) {
        EXCEPTION_NESTED("Failed to unpack message of type '{}'", typeid(T).name());
    }
    return ret;
}
}; // namespace Engine
