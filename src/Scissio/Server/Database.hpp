#pragma once

#include "../Utils/Exceptions.hpp"
#include "../Utils/Msgpack.hpp"
#include "../Utils/Path.hpp"
#include <functional>
#include <optional>

namespace Scissio {
class Database {
public:
    explicit Database(const Path& path);
    ~Database();

    template <typename T> std::optional<T> get(const std::string& key) {
        T value;

        const auto handler = [&](const char* data, const size_t size) {
            try {
                msgpack::unpacked result;
                msgpack::unpack(result, data, size);
                msgpack::object obj(result.get());
                obj.convert(value);
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: {} as: {}", key, typeid(T).name());
            }
        };

        if (get(key, handler)) {
            return value;
        }
        return std::nullopt;
    }

    template <typename T> void put(const std::string& key, const T& value) {
        msgpack::sbuffer sbuf;
        try {
            msgpack::packer<msgpack::sbuffer> packer(sbuf);
            msgpack::pack(sbuf, value);
        } catch (...) {
            EXCEPTION_NESTED("Failed to pack database key: {} as: {}", key, typeid(T).name());
        }

        put(key, sbuf.data(), sbuf.size());
    }

    template <typename T> bool update(const std::string& key, const std::function<std::optional<T>(const T&)>& fn) {
        const auto handler = [&](const char* data, const size_t size,
                                 const std::function<void(const char*, size_t)>& callback) {
            T value;

            try {
                msgpack::unpacked result;
                msgpack::unpack(result, data, size);
                msgpack::object obj(result.get());
                obj.convert(value);
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: {} as: {}", key, typeid(T).name());
            }

            auto res = fn(value);
            if (res.has_value()) {
                msgpack::sbuffer sbuf;

                try {
                    msgpack::packer<msgpack::sbuffer> packer(sbuf);
                    msgpack::pack(sbuf, value);
                } catch (...) {
                    EXCEPTION_NESTED("Failed to pack database key: {} as: {}", key, typeid(T).name());
                }

                callback(sbuf.data(), sbuf.size());
            }
        };

        return update(key, handler);
    }

    void remove(const std::string& key);

private:
    bool get(const std::string& key, const std::function<void(const char*, size_t)>& fn);
    bool update(const std::string& key,
                const std::function<void(const char*, size_t, const std::function<void(const char*, size_t)>&)>& fn);
    void put(const std::string& key, const char* rawData, size_t size);

    struct Data;

    std::unique_ptr<Data> data;
};
} // namespace Scissio
