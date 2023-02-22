#pragma once

#include "../library.hpp"
#include "../utils/exceptions.hpp"
#include "../utils/macros.hpp"
#include "../utils/msgpack_adaptors.hpp"
#include "../utils/path.hpp"
#include "../utils/span.hpp"
#include <functional>
#include <msgpack.hpp>
#include <optional>

namespace Engine {
class ENGINE_API Database {
public:
    using Callback = std::function<void(const std::string& key, const Span<char>& data)>;

    template <typename T> struct SchemaSelf {
        using Self = T;
    };

    template <typename... Schemas> struct SchemaVersioning;
    template <typename Schema> struct SchemaVersion {
        static size_t getVersion() {
            return 1ULL;
        }
    };

    template <typename T> struct SchemaUnpacker {
        static void unpack(const msgpack::object& obj, T& value, const size_t version) {
            if (version != 1ULL) {
                EXCEPTION("Unable to unpack schema: '{}' with version: '{}' error: Only version 1 defined as default",
                          T::getSchemaName());
            }
            obj.convert(value);
        }
    };

    template <typename Schema> struct SchemaVersioning<Schema> {
        template <typename T> static size_t getVersion(size_t version = 0);
        template <typename T>
        static void unpack(const msgpack::object& obj, T& value, size_t version, size_t current = 1);
    };

    template <typename Schema, typename... Schemas> struct SchemaVersioning<Schema, Schemas...> {
        template <typename T> static size_t getVersion(size_t version = 0);
        template <typename T>
        static void unpack(const msgpack::object& obj, T& value, size_t version, size_t current = 1);
    };

    template <typename T> struct SchemaGetVarClass {};

    template <typename Class, typename Value> struct SchemaGetVarClass<Value Class::*> {
        using type = Class;
    };

    template <typename M> struct SchemaGetVarType {
        template <typename C, typename T> static T getType(T C::*v) {
        }

        typedef decltype(getType(static_cast<M>(nullptr))) type;
    };

    template <typename T> static std::string keyToSchemaDataKey(const std::string_view& key) {
        return fmt::format("{}:data:{}", T::getSchemaName(), key);
    }

    template <typename T, auto F> static const char* getSchemaIndexName() {
        return T::template getIndexName<F>();
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    static std::string keyToSchemaIndexKey() {
        return fmt::format("{}:index:{}:", T::getSchemaName(), getSchemaIndexName<T, F>());
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    static std::string keyToSchemaIndexKey(const R& value) {
        return fmt::format("{}:index:{}:{}:", T::getSchemaName(), getSchemaIndexName<T, F>(), value);
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    static std::string keyToSchemaIndexKey(const R& value, const std::string& key) {
        return fmt::format("{}:index:{}:{}:{}", T::getSchemaName(), getSchemaIndexName<T, F>(), value, key);
    }

    template <typename T> static size_t getSchemaVersion() {
        return SchemaVersion<T>::getVersion();
    }

    template <typename T> static msgpack::sbuffer packSchema(const T& value);
    template <typename T> static void unpackSchema(const Span<char>& data, T& value);

    class InternalIterator {
    public:
        virtual ~InternalIterator() = default;
        virtual bool next(const Callback& callback) = 0;
    };

    template <typename T> class Iterator {
    public:
        explicit Iterator(std::unique_ptr<InternalIterator> it) : it(std::move(it)) {
            next();
        }

        operator bool() const {
            return bool(it);
        }

        void next() {
            const auto handler = [this](const std::string& k, const Span<char>& data) {
                key = k;
                unpackSchema(data, value);
            };

            if (it && !it->next(handler)) {
                it.reset();
            }
        }

        std::string key;
        T value{};

    private:
        std::unique_ptr<InternalIterator> it;
    };

    virtual ~Database() = default;
    template <typename T> std::optional<T> get(const std::string& key);
    template <typename T> void put(const std::string& key, const T& value);
    template <typename T> bool remove(const std::string& key);
    template <typename T> void removeByPrefix(const std::string& key);
    template <typename T> std::vector<T> multiGet(const std::vector<std::string>& keys);
    template <typename T>
    Iterator<T> seek(const std::string& key, const std::optional<std::string>& lowerBound = std::nullopt);
    template <typename T> std::vector<T> seekAll(const std::string& key, const size_t max = 0);
    template <typename T>
    std::vector<T> next(const std::string& prefix, const std::string& start, const size_t max = 0,
                        std::string* lastKey = nullptr);

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    void putIndex(const std::string& key, const R& value) {
        const auto fullIndexKey = keyToSchemaIndexKey<F>(value, key);
        const auto fullKey = keyToSchemaDataKey<T>(key);
        internalPut(fullIndexKey, {fullKey.data(), fullKey.size()});
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    void removeIndex(const std::string& key, const R& value) {
        const auto fullIndexKey = keyToSchemaIndexKey<F>(value, key);
        internalRemove(fullIndexKey);
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    std::vector<T> getByIndex(const R& value);

private:
    virtual void internalGet(const std::string& key, const Callback& callback) = 0;
    virtual void internalMultiGet(const std::vector<std::string>& keys, const Callback& callback) = 0;
    virtual void internalPut(const std::string& key, const Span<char>& data) = 0;
    virtual void internalRemove(const std::string& key) = 0;
    virtual std::unique_ptr<InternalIterator> internalSeek(const std::string& key,
                                                           const std::optional<std::string>& lowerBound) = 0;
};

template <> class Database::Iterator<std::string> {
public:
    explicit Iterator(std::unique_ptr<InternalIterator> it) : it(std::move(it)) {
        next();
    }

    operator bool() const {
        return bool(it);
    }

    void next() {
        const auto handler = [this](const std::string& k, const Span<char>& data) {
            key = k;
            value = std::string(data.data(), data.size());
        };

        if (it && !it->next(handler)) {
            it.reset();
        }
    }

    std::string key;
    std::string value{};

private:
    std::unique_ptr<InternalIterator> it;
};

class ENGINE_API Transaction : public Database {
public:
    virtual ~Transaction() = default;

    template <typename T> std::optional<T> getForUpdate(const std::string& key);

private:
    virtual void internalGetForUpdate(const std::string& key, const Callback& callback) = 0;
};

class ENGINE_API TransactionalDatabase : public Database {
public:
    virtual ~TransactionalDatabase() = default;

    virtual bool transaction(const std::function<bool(Transaction&)>& callback, bool retry = true) = 0;
    template <typename T>
    std::tuple<bool, std::optional<T>> update(const std::string& key,
                                              const std::function<bool(std::optional<T>&)>& callback);
};

template <typename Schema>
template <typename T>
size_t Database::SchemaVersioning<Schema>::getVersion(const size_t version) {
    static_assert(std::is_same<Schema, T>::value, "Schema type should match T type");
    return version + 1ULL;
}

template <typename Schema, typename... Schemas>
template <typename T>
size_t Database::SchemaVersioning<Schema, Schemas...>::getVersion(const size_t version) {
    if constexpr (std::is_same<Schema, T>::value) {
        return version + 1ULL;
    } else {
        return Database::SchemaVersioning<Schemas...>::template getVersion<T>(version + 1ULL);
    }
}

template <typename Schema>
template <typename T>
void Database::SchemaVersioning<Schema>::unpack(const msgpack::object& obj, T& value, const size_t version,
                                                size_t current) {
    if constexpr (std::is_same<Schema, T>::value) {
        if (version > current) {
            EXCEPTION("Invalid version for schema: '{}' version: {}, can not convert from higher to lower",
                      T::getSchemaName(), version);
        }
        obj.convert(value);
    } else {
        EXCEPTION("Invalid version for schema: '{}' version: {}", T::getSchemaName(), version);
    }
}

template <typename Schema, typename... Schemas>
template <typename T>
void Database::SchemaVersioning<Schema, Schemas...>::unpack(const msgpack::object& obj, T& value, const size_t version,
                                                            size_t current) {
    if constexpr (std::is_same<Schema, T>::value) {
        if (version > current) {
            EXCEPTION("Invalid version for schema: '{}' version: {}, can not convert from higher to lower",
                      T::getSchemaName(), version);
        }
        obj.convert(value);
    } else {
        if (version == current) {
            Schema temp{};
            obj.convert(temp);
            value = std::move(temp);
        } else if (version > current) {
            SchemaVersioning<Schemas...>::unpack(obj, value, version, current + 1);
        } else {
            EXCEPTION("Invalid version for schema: '{}' version: {}", T::getSchemaName(), version);
        }
    }
}

template <typename T> msgpack::sbuffer Database::packSchema(const T& value) {
    try {
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_array(2);
        packer.pack_uint64(getSchemaVersion<T>());
        packer.pack(value);
        return sbuf;
    } catch (...) {
        EXCEPTION_NESTED("Failed to pack schema: {}", T::getSchemaName());
    }
}

template <typename T> void Database::unpackSchema(const Span<char>& data, T& value) {
    msgpack::unpacked result;
    msgpack::unpack(result, data.data(), data.size());
    msgpack::object obj(result.get());

    if (obj.type != msgpack::type::ARRAY) {
        EXCEPTION("Object is not an array");
    }
    if (obj.via.array.size != 2) {
        EXCEPTION("Object is not an array of 2 items");
    }
    size_t version = 0;
    obj.via.array.ptr[0].convert(version);

    SchemaUnpacker<T>::unpack(obj.via.array.ptr[1], value, version);
}

#define SCHEMA(Name) struct Name : Engine::Database::SchemaSelf<Name>

#define SCHEMA_EXPAND(x) x
#define SCHEMA_UNPACK(...) __VA_ARGS__
#define SCHEMA_FOR_EACH_NARG(...) SCHEMA_FOR_EACH_NARG_(__VA_ARGS__, SCHEMA_FOR_EACH_RSEQ_N())
#define SCHEMA_FOR_EACH_NARG_(...) SCHEMA_EXPAND(SCHEMA_FOR_EACH_ARG_N(__VA_ARGS__))
#define SCHEMA_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define SCHEMA_FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define SCHEMA_CONCATENATE(x, y) x##y
#define SCHEMA_MACRO_CONCAT(x, y) SCHEMA_CONCATENATE(x, y)

#define SCHEMA_VERSION(N, U, S)                                                                                        \
    template <> struct Engine::Database::SchemaVersion<S> {                                                            \
        using Versioning = U;                                                                                          \
        static size_t getVersion() {                                                                                   \
            return U::getVersion<S>();                                                                                 \
        }                                                                                                              \
    };                                                                                                                 \
    template <> struct Engine::Database::SchemaUnpacker<S> {                                                           \
        static void unpack(const msgpack::object& obj, S& value, size_t version) {                                     \
            return U::unpack<S>(obj, value, version);                                                                  \
        }                                                                                                              \
    };

#define SCHEMA_VERSIONING_1(N, U, S, ...) SCHEMA_VERSION(N - 0, U, S)

#define SCHEMA_VERSIONING_2(N, U, S, ...)                                                                              \
    SCHEMA_VERSION(N - 1, U, S)                                                                                        \
    SCHEMA_VERSIONING_1(N, U, __VA_ARGS__)

#define SCHEMA_VERSIONING_3(N, U, S, ...)                                                                              \
    SCHEMA_VERSION(N - 2, U, S)                                                                                        \
    SCHEMA_VERSIONING_2(N, U, __VA_ARGS__)

#define SCHEMA_VERSIONING_4(N, U, S, ...)                                                                              \
    SCHEMA_VERSION(N - 3, U, S)                                                                                        \
    SCHEMA_VERSIONING_3(N, U, __VA_ARGS__)

#define SCHEMA_VERSIONING_(N, U, ...) SCHEMA_EXPAND(SCHEMA_CONCATENATE(SCHEMA_VERSIONING_, N)(N, U, __VA_ARGS__))
#define SCHEMA_VERSIONING(U, ...) SCHEMA_VERSIONING_(SCHEMA_FOR_EACH_NARG(__VA_ARGS__), U, __VA_ARGS__)

#define SCHEMA_NAME(Name)                                                                                              \
    static inline std::string_view getSchemaName() {                                                                   \
        return Name;                                                                                                   \
    }

#define SCHEMA_DEFINE(...) MSGPACK_DEFINE_MAP(__VA_ARGS__)

#define SCHEMA_VERSIONS_IMPL(counter, ...)                                                                             \
    using SCHEMA_MACRO_CONCAT(DatabaseSchemaVersioning, counter) =                                                     \
        Engine::Database::SchemaVersioning<SCHEMA_UNPACK(__VA_ARGS__)>;                                                \
    SCHEMA_VERSIONING(SCHEMA_MACRO_CONCAT(DatabaseSchemaVersioning, counter), __VA_ARGS__)
#define SCHEMA_VERSIONS(...) SCHEMA_VERSIONS_IMPL(__COUNTER__, __VA_ARGS__)

#define SCHEMA_FOR_EACH_INDEX_PUT_FIELD(Field) db.putIndex<&Self::Field>(key, value.Field);
#define SCHEMA_FOR_EACH_INDEX_PUT(...) FOR_EACH(SCHEMA_FOR_EACH_INDEX_PUT_FIELD, __VA_ARGS__)
#define SCHEMA_FOR_EACH_INDEX_REMOVE_FIELD(Field) db.removeIndex<&Self::Field>(key, value.Field);
#define SCHEMA_FOR_EACH_INDEX_REMOVE(...) FOR_EACH(SCHEMA_FOR_EACH_INDEX_REMOVE_FIELD, __VA_ARGS__)

#define SCHEMA_FOR_EACH_INDEX_NAME_FIELD(Field)                                                                        \
    if (reinterpret_cast<int Self::*>(F) == reinterpret_cast<int Self::*>(&Self::Field))                               \
        return #Field;

#define SCHEMA_FOR_EACH_INDEX_NAME(...) FOR_EACH(SCHEMA_FOR_EACH_INDEX_NAME_FIELD, __VA_ARGS__)

#define SCHEMA_INDEXES(...)                                                                                            \
    template <typename Class, typename T, T Class::*Field> struct SchemaIndexDefinition {                              \
        static const char* getName();                                                                                  \
    };                                                                                                                 \
    template <auto F> static const char* getIndexName() {                                                              \
        SCHEMA_FOR_EACH_INDEX_NAME(__VA_ARGS__)                                                                        \
        EXCEPTION("No such index defined as: '{}'", typeid(decltype(F)).name());                                       \
    }                                                                                                                  \
    static void putIndexes(Engine::Database& db, const Self& value, const std::string& key) {                          \
        SCHEMA_FOR_EACH_INDEX_PUT(__VA_ARGS__)                                                                         \
    }                                                                                                                  \
    static void removeIndexes(Engine::Database& db, const Self& value, const std::string& key) {                       \
        SCHEMA_FOR_EACH_INDEX_REMOVE(__VA_ARGS__)                                                                      \
    }

#define SCHEMA_INDEXES_NONE()                                                                                          \
    static void putIndexes(Engine::Database& db, const Self& value, const std::string& key) {                          \
        (void)db;                                                                                                      \
        (void)value;                                                                                                   \
        (void)key;                                                                                                     \
    }                                                                                                                  \
    static void removeIndexes(Engine::Database& db, const Self& value, const std::string& key) {                       \
        (void)db;                                                                                                      \
        (void)value;                                                                                                   \
        (void)key;                                                                                                     \
    }

template <typename T> std::optional<T> Database::get(const std::string& key) {
    try {
        T value;
        bool found = false;

        const auto handler = [&](const std::string&, const Span<char>& data) {
            Database::unpackSchema(data, value);
            found = true;
        };

        const auto fullKey = Database::keyToSchemaDataKey<T>(key);
        internalGet(fullKey, handler);

        if (found) {
            return value;
        }

        return std::nullopt;
    } catch (...) {
        EXCEPTION_NESTED("Failed to get database key: {} as: {}", key, T::getSchemaName());
    }
}

template <typename T> std::vector<T> Database::multiGet(const std::vector<std::string>& keys) {
    try {
        std::vector<T> values;

        const auto handler = [&](const std::string& key, const Span<char>& data) {
            values.emplace_back();
            try {
                Database::unpackSchema(data, values.back());
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: '{}' as: {}", key, T::getSchemaName());
            }
        };

        std::vector<std::string> keysInternal;
        keysInternal.reserve(keys.size());
        for (const auto& key : keys) {
            keysInternal.push_back(Database::keyToSchemaDataKey<T>(key));
        }
        internalMultiGet(keysInternal, handler);

        return values;
    } catch (...) {
        EXCEPTION_NESTED("Failed to multi-get database key as: {}", T::getSchemaName());
    }
}

template <typename T> void Database::put(const std::string& key, const T& value) {
    try {
        const auto sbuf = Database::packSchema(value);
        const auto fullKey = Database::keyToSchemaDataKey<T>(key);
        internalPut(fullKey, {sbuf.data(), sbuf.size()});
        T::putIndexes(*this, value, key);
    } catch (...) {
        EXCEPTION_NESTED("Failed to put database key: '{}' as: {}", key, T::getSchemaName());
    }
}

template <typename T> bool Database::remove(const std::string& key) {
    try {
        const auto found = get<T>(key);
        if (found.has_value()) {
            const auto fullKey = Database::keyToSchemaDataKey<T>(key);
            internalRemove(fullKey);
            T::removeIndexes(*this, found.value(), key);
            return true;
        } else {
            return false;
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to delete database key: '{}' as: {}", key, T::getSchemaName());
    }
}

template <typename T>
Database::Iterator<T> Database::seek(const std::string& key, const std::optional<std::string>& lowerBound) {
    try {
        const auto fullKey = Database::keyToSchemaDataKey<T>(key);
        std::optional<std::string> fullLowerBound;
        if (lowerBound.has_value()) {
            fullLowerBound = Database::keyToSchemaDataKey<T>(lowerBound.value());
        }
        return Iterator<T>(internalSeek(fullKey, fullLowerBound));
    } catch (...) {
        EXCEPTION_NESTED("Failed to create database seek iterator key: '{}' as: {}", key, T::getSchemaName());
    }
}

template <typename T> void Database::removeByPrefix(const std::string& key) {
    try {
        auto it = seek<T>(key);
        while (it) {
            internalRemove(it.key);
            T::removeIndexes(*this, it.value, it.key);
            it.next();
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to remove database keys by prefix: '{}' as: {}", key, T::getSchemaName());
    }
}

template <typename T> std::vector<T> Database::seekAll(const std::string& key, const size_t max) {
    std::vector<T> items;
    auto it = seek<T>(key);
    while (it) {
        items.emplace_back();
        std::swap(items.back(), it.value);

        if (items.size() == max) {
            break;
        }

        it.next();
    }
    return items;
}

template <typename T>
std::vector<T> Database::next(const std::string& prefix, const std::string& start, const size_t max,
                              std::string* lastKey) {
    std::vector<T> items;
    const auto substrLength = T::getSchemaName().size() + 6; // + strlen(":data:")
    const auto keyStart = Database::keyToSchemaDataKey<T>(start);
    const auto keyPrefix = Database::keyToSchemaDataKey<T>(prefix);

    auto it = seek<T>(prefix, start);
    while (it) {
        if (it.key == keyStart || it.key.find(keyPrefix) != 0) {
            it.next();
            continue;
        }

        items.emplace_back();
        std::swap(items.back(), it.value);

        if (items.size() == max) {
            if (lastKey) {
                *lastKey = it.key.substr(substrLength);
            }
            break;
        }

        it.next();
    }

    return items;
}

template <auto F, typename T, typename R> std::vector<T> Database::getByIndex(const R& value) {
    try {
        const auto fullIndexKey = keyToSchemaIndexKey<F>(value);

        std::vector<T> items;

        const auto handler = [&](const std::string& key, const Span<char>& data) {
            items.emplace_back();
            Database::unpackSchema(data, items.back());
        };

        auto it = Iterator<std::string>(internalSeek(fullIndexKey, std::nullopt));
        while (it) {
            internalGet(it.value, handler);
            it.next();
        }

        return items;
    } catch (...) {
        EXCEPTION_NESTED("Failed to get database values by index: '{}' value: '{}' as: {}", typeid(decltype(F)).name(),
                         value, T::getSchemaName());
    }
}

template <typename T> std::optional<T> Transaction::getForUpdate(const std::string& key) {
    try {
        T value;
        bool found = false;

        const auto handler = [&](const std::string&, const Span<char>& data) {
            Database::unpackSchema(data, value);
            found = true;
        };

        const auto fullKey = keyToSchemaDataKey<T>(key);
        internalGetForUpdate(fullKey, handler);

        if (found) {
            return value;
        }

        return std::nullopt;
    } catch (...) {
        EXCEPTION_NESTED("Failed to get database key: {} as: {}", key, T::getSchemaName());
    }
}

template <typename T>
std::tuple<bool, std::optional<T>>
TransactionalDatabase::update(const std::string& key, const std::function<bool(std::optional<T>&)>& callback) {
    std::optional<T> found;
    const auto updated = transaction([&](Transaction& txn) -> bool {
        found = txn.template getForUpdate<T>(key);

        const auto res = callback(found);
        if (res && found.has_value()) {
            txn.template put<T>(key, found.value());
            return true;
        }

        return false;
    });

    return {updated, found};
}
} // namespace Engine
