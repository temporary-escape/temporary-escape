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
class ENGINE_API DatabaseBackendOperations;
class ENGINE_API DatabaseOperations;

namespace Details {
template <typename T> struct SchemaSelf {
    using Self = T;
};

template <typename... Schemas> struct SchemaVersioning;
template <typename Schema> struct SchemaVersion {
    static size_t getVersion() {
        return 1ULL;
    }
};

template <typename T> std::string_view getSchemaName() {
    return T::getSchemaName();
}

template <typename T> struct SchemaUnpacker {
    static void unpack(const msgpack::object& obj, T& value, const size_t version) {
        if (version != 1ULL) {
            throw std::runtime_error(
                fmt::format("Unable to unpack schema: '{}' with version: '{}' error: Only version 1 defined as default",
                            getSchemaName<T>()));
        }
        obj.convert(value);
    }
};

template <typename Schema> struct SchemaVersioning<Schema> {
    template <typename T> static size_t getVersion(size_t version = 0);
    template <typename T> static void unpack(const msgpack::object& obj, T& value, size_t version, size_t current = 1);
};

template <typename Schema, typename... Schemas> struct SchemaVersioning<Schema, Schemas...> {
    template <typename T> static size_t getVersion(size_t version = 0);
    template <typename T> static void unpack(const msgpack::object& obj, T& value, size_t version, size_t current = 1);
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
static std::string keyToSchemaIndexKey(const R& value, const std::string_view& key) {
    return fmt::format("{}:index:{}:{}:{}", T::getSchemaName(), getSchemaIndexName<T, F>(), value, key);
}

template <typename T> static size_t getSchemaVersion() {
    return SchemaVersion<T>::getVersion();
}

template <typename T> msgpack::sbuffer packSchema(const T& value);
template <typename T> void unpackSchema(const msgpack::object& obj, T& value);
template <typename T> void putIndexes(DatabaseOperations& backend, const std::string_view& key, const T& value);
template <typename T> void removeIndexes(DatabaseOperations& backend, const std::string_view& key, const T& value);
} // namespace Details

class DatabaseBackendIterator {
public:
    virtual ~DatabaseBackendIterator() = default;

    virtual bool next() = 0;
    virtual operator bool() const = 0;
    [[nodiscard]] virtual const std::string& key() const = 0;
    [[nodiscard]] virtual const msgpack::object_handle& value() const = 0;
    [[nodiscard]] virtual std::string valueRaw() const = 0;
};

class DatabaseBackendOperations {
public:
    virtual ~DatabaseBackendOperations() = default;

    virtual std::optional<msgpack::object_handle> get(const std::string_view& key) = 0;
    virtual std::vector<msgpack::object_handle> multiGet(const std::vector<std::string>& keys) = 0;
    virtual void put(const std::string_view& key, const void* data, size_t size) = 0;
    virtual void remove(const std::string_view& key) = 0;
    virtual std::unique_ptr<DatabaseBackendIterator> seek(const std::string_view& prefix,
                                                          const std::optional<std::string_view>& lowerBound) = 0;
};

class ENGINE_API DatabaseBackendTransaction : public DatabaseBackendOperations {
public:
    ~DatabaseBackendTransaction() override = default;
    virtual std::optional<msgpack::object_handle> getForUpdate(const std::string_view& key) = 0;
};

class ENGINE_API DatabaseBackend : public DatabaseBackendOperations {
public:
    ~DatabaseBackend() override = default;
    virtual bool transaction(const std::function<bool(DatabaseBackendTransaction&)>& callback, bool retry) = 0;
};

template <typename T> class DatabaseIterator {
public:
    explicit DatabaseIterator(std::unique_ptr<DatabaseBackendIterator> backend) : backend{std::move(backend)} {
    }
    virtual ~DatabaseIterator() = default;
    DatabaseIterator(const DatabaseIterator& other) = delete;
    DatabaseIterator(DatabaseIterator&& other) noexcept = default;
    DatabaseIterator& operator=(const DatabaseIterator& other) = delete;
    DatabaseIterator& operator=(DatabaseIterator&& other) noexcept = default;

    bool next() {
        return backend->next();
    }

    [[nodiscard]] const std::string& key() const {
        return backend->key();
    }

    T value() const {
        T res{};
        unpackSchema(backend->value().get(), res);
        return res;
    }

private:
    std::unique_ptr<DatabaseBackendIterator> backend;
};

class DatabaseOperations {
public:
    explicit DatabaseOperations(DatabaseBackendOperations& backend) : backend{backend} {
    }

    virtual ~DatabaseOperations() = default;

    template <typename T> std::optional<T> find(const std::string_view& key);
    template <typename T> T get(const std::string_view& key);
    template <typename T> void put(const std::string_view& key, const T& value);
    template <typename T> std::vector<T> multiGet(const std::vector<std::string>& keys);
    template <typename T> bool remove(const std::string_view& key);

    template <auto F, typename T = typename Details::SchemaGetVarClass<decltype(F)>::type,
              typename R = typename Details::SchemaGetVarType<decltype(F)>::type>
    void putIndex(const std::string_view& key, const R& value);

    template <auto F, typename T = typename Details::SchemaGetVarClass<decltype(F)>::type,
              typename R = typename Details::SchemaGetVarType<decltype(F)>::type>
    void removeIndex(const std::string_view& key, const R& value);
    template <typename T>
    DatabaseIterator<T> seek(const std::string_view& prefix, const std::string_view& lowerBound = "");
    template <typename T> void removeByPrefix(const std::string_view& key);
    template <typename T> std::vector<T> seekAll(const std::string& key, size_t max = 0);
    template <typename T>
    std::vector<T> next(const std::string_view& prefix, const std::string_view& start, size_t max,
                        std::string* lastKey);
    template <auto F, typename T = typename Details::SchemaGetVarClass<decltype(F)>::type,
              typename R = typename Details::SchemaGetVarType<decltype(F)>::type>
    std::vector<T> getByIndex(const R& value);

private:
    DatabaseBackendOperations& backend;
};

class DatabaseTransaction : public DatabaseOperations {
public:
    explicit DatabaseTransaction(DatabaseBackendTransaction& backend) : DatabaseOperations{backend}, backend{backend} {
    }
    ~DatabaseTransaction() override = default;

    template <typename T> std::optional<T> getForUpdate(const std::string_view& key);

private:
    DatabaseBackendTransaction& backend;
};

class ENGINE_API Database : public DatabaseOperations {
public:
    using TransactionCallback = std::function<bool(DatabaseTransaction&)>;

    explicit Database(DatabaseBackend& backend) : DatabaseOperations{backend}, backend{backend} {
    }

    ~Database() override = default;

    bool transaction(const TransactionCallback& callback, bool retry = true) {
        return backend.transaction(
            [&](DatabaseBackendTransaction& backendTransaction) -> bool {
                DatabaseTransaction txd{backendTransaction};
                return callback(txd);
            },
            retry);
    }
    template <typename T> T update(const std::string_view& key, const std::function<T(std::optional<T>)>& callback);

private:
    DatabaseBackend& backend;
};

template <typename T> std::optional<T> DatabaseOperations::find(const std::string_view& key) {
    try {
        const auto fullKey = Details::keyToSchemaDataKey<T>(key);
        const auto object = backend.get(fullKey);

        if (object) {
            T value{};
            Details::unpackSchema(object->get(), value);
            return value;
        }

        return {};
    } catch (std::exception& e) {
        throw std::runtime_error(
            fmt::format("Failed to get database key: {} as: {} error: {}", key, Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T> T DatabaseOperations::get(const std::string_view& key) {
    auto item = this->template find<T>(key);
    if (!item) {
        EXCEPTION("No such database key: {} as: {}", key, T::getSchemaName());
    }
    return std::move(item.value());
}

template <typename T> std::optional<T> DatabaseTransaction::getForUpdate(const std::string_view& key) {
    try {
        const auto fullKey = Details::keyToSchemaDataKey<T>(key);
        const auto object = backend.getForUpdate(fullKey);

        if (object) {
            T value{};
            unpackSchema(object->get(), value);
            return value;
        }

        return {};
    } catch (std::exception& e) {
        throw std::runtime_error(
            fmt::format("Failed to get database key: {} as: {} error: {}", key, Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T> void DatabaseOperations::put(const std::string_view& key, const T& value) {
    try {
        const auto sbuf = Details::packSchema<T>(value);
        const auto fullKey = Details::keyToSchemaDataKey<T>(key);

        backend.put(fullKey, sbuf.data(), sbuf.size());
        Details::putIndexes<T>(*this, key, value);

    } catch (std::exception& e) {
        throw std::runtime_error(
            fmt::format("Failed to put database key: {} as: {} error: {}", key, Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T> std::vector<T> DatabaseOperations::multiGet(const std::vector<std::string>& keys) {
    try {
        std::vector<T> values;

        std::vector<std::string> keysInternal;
        keysInternal.reserve(keys.size());
        for (const auto& key : keys) {
            keysInternal.push_back(Details::keyToSchemaDataKey<T>(key));
        }

        const auto objects = backend.multiGet(keysInternal);
        values.resize(objects.size());

        for (size_t i = 0; i < values.size(); i++) {
            unpackSchema(objects.at(i).get(), values.at(i));
        }

        return values;
    } catch (std::exception& e) {
        throw std::runtime_error(
            fmt::format("Failed to multi-get database as: {} error: {}", Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T> bool DatabaseOperations::remove(const std::string_view& key) {
    try {
        const auto found = find<T>(key);
        if (found.has_value()) {
            const auto fullKey = Details::keyToSchemaDataKey<T>(key);
            backend.remove(fullKey);
            Details::removeIndexes<T>(*this, key, found.value());
            return true;
        } else {
            return false;
        }
    } catch (std::exception& e) {
        throw std::runtime_error(fmt::format(
            "Failed to delete database key: {} as: {} error: {}", key, Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T>
DatabaseIterator<T> DatabaseOperations::seek(const std::string_view& prefix, const std::string_view& lowerBound) {
    const auto fullKey = Details::keyToSchemaDataKey<T>(prefix);
    std::optional<std::string> fullLowerBound;
    if (!lowerBound.empty()) {
        fullLowerBound = Details::keyToSchemaDataKey<T>(lowerBound);
    }
    return DatabaseIterator<T>(backend.seek(fullKey, fullLowerBound));
}

template <typename T> void DatabaseOperations::removeByPrefix(const std::string_view& key) {
    try {
        auto it = this->template seek<T>(key);
        while (it.next()) {
            backend.remove(it.key());
            Details::removeIndexes<T>(*this, it.key(), it.value());
        }
    } catch (std::exception& e) {
        throw std::runtime_error(fmt::format(
            "Failed to delete database keys prefix: {} as: {} error: {}", key, Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T> std::vector<T> DatabaseOperations::seekAll(const std::string& key, const size_t max) {
    std::vector<T> items;
    auto it = this->template seek<T>(key);
    while (it.next()) {
        items.push_back(it.value());
        if (items.size() == max) {
            break;
        }
    }
    return items;
}

template <typename T>
std::vector<T> DatabaseOperations::next(const std::string_view& prefix, const std::string_view& start, const size_t max,
                                        std::string* lastKey) {
    std::vector<T> items;
    const auto substrLength = T::getSchemaName().size() + 6; // + strlen(":data:")
    const auto keyStart = Details::keyToSchemaDataKey<T>(start);
    const auto keyPrefix = Details::keyToSchemaDataKey<T>(prefix);

    auto it = this->template seek<T>(prefix, start);
    while (it.next()) {
        if (it.key() == keyStart || it.key().find(keyPrefix) != 0) {
            continue;
        }

        items.push_back(it.value());

        if (items.size() == max) {
            if (lastKey) {
                *lastKey = it.key().substr(substrLength);
            }
            break;
        }
    }

    return items;
}

template <auto F, typename T, typename R> std::vector<T> DatabaseOperations::getByIndex(const R& value) {
    try {
        const auto fullIndexKey = Details::keyToSchemaIndexKey<F>(value);

        std::vector<T> items;

        auto it = backend.seek(fullIndexKey, std::nullopt);
        while (it->next()) {
            const auto key = it->valueRaw();
            const auto item = backend.get(key);

            if (item) {
                items.emplace_back();
                unpackSchema(item->get(), items.back());
            }
        }

        return items;
    } catch (std::exception& e) {
        throw std::runtime_error(fmt::format("Failed to get database values by index: {} value: {} as: {} error: {}",
                                             typeid(decltype(F)).name(),
                                             value,
                                             Details::getSchemaName<T>(),
                                             e.what()));
    }
}

template <typename T>
T Database::update(const std::string_view& key, const std::function<T(std::optional<T>)>& callback) {
    std::optional<T> found;
    transaction([&](DatabaseTransaction& txn) -> bool {
        found = txn.template getForUpdate<T>(key);
        found = callback(std::move(found));
        txn.template put<T>(key, found.value());
        return true;
    });

    if (!found) {
        throw std::runtime_error(fmt::format(
            "Failed to update database key: {} as: {} error: transaction failed", key, Details::getSchemaName<T>()));
    }

    return found.value();
}

template <typename Schema>
template <typename T>
size_t Details::SchemaVersioning<Schema>::getVersion(const size_t version) {
    static_assert(std::is_same<Schema, T>::value, "Schema type should match T type");
    return version + 1ULL;
}

template <typename Schema, typename... Schemas>
template <typename T>
size_t Details::SchemaVersioning<Schema, Schemas...>::getVersion(const size_t version) {
    if constexpr (std::is_same<Schema, T>::value) {
        return version + 1ULL;
    } else {
        return Details::SchemaVersioning<Schemas...>::template getVersion<T>(version + 1ULL);
    }
}

template <typename Schema>
template <typename T>
void Details::SchemaVersioning<Schema>::unpack(const msgpack::object& obj, T& value, const size_t version,
                                               size_t current) {
    if constexpr (std::is_same<Schema, T>::value) {
        if (version > current) {
            EXCEPTION("Invalid version for schema: '{}' version: {}, can not convert from higher to lower",
                      T::getSchemaName(),
                      version);
        }
        obj.convert(value);
    } else {
        EXCEPTION("Invalid version for schema: '{}' version: {}", T::getSchemaName(), version);
    }
}

template <typename Schema, typename... Schemas>
template <typename T>
void Details::SchemaVersioning<Schema, Schemas...>::unpack(const msgpack::object& obj, T& value, const size_t version,
                                                           size_t current) {
    if constexpr (std::is_same<Schema, T>::value) {
        if (version > current) {
            EXCEPTION("Invalid version for schema: '{}' version: {}, can not convert from higher to lower",
                      T::getSchemaName(),
                      version);
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
            throw std::runtime_error(
                fmt::format("Invalid version for schema: '{}' version: {}", Details::getSchemaName<T>(), version));
        }
    }
}

template <typename T> msgpack::sbuffer Details::packSchema(const T& value) {
    try {
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_array(2);
        packer.pack_uint64(getSchemaVersion<T>());
        packer.pack(value);
        return sbuf;
    } catch (std::exception& e) {
        throw std::runtime_error(fmt::format("Failed to pack schema: {}", Details::getSchemaName<T>(), e.what()));
    }
}

template <typename T> void Details::unpackSchema(const msgpack::object& obj, T& value) {
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

template <auto F, typename T, typename R>
void DatabaseOperations::putIndex(const std::string_view& key, const R& value) {
    const auto fullIndexKey = Details::keyToSchemaIndexKey<F>(value, key);
    const auto fullKey = Details::keyToSchemaDataKey<T>(key);
    backend.put(fullIndexKey, fullKey.data(), fullKey.size());
}

template <auto F, typename T, typename R>
void DatabaseOperations::removeIndex(const std::string_view& key, const R& value) {
    const auto fullIndexKey = Details::keyToSchemaIndexKey<F>(value, key);
    backend.remove(fullIndexKey);
}

template <typename T> void Details::putIndexes(DatabaseOperations& db, const std::string_view& key, const T& value) {
    T::putIndexes(db, key, value);
}

template <typename T> void Details::removeIndexes(DatabaseOperations& db, const std::string_view& key, const T& value) {
    T::removeIndexes(db, key, value);
}

#define SCHEMA(Name) struct Name : Engine::Details::SchemaSelf<Name>

#define SCHEMA_EXPAND(x) x
#define SCHEMA_UNPACK(...) __VA_ARGS__
#define SCHEMA_FOR_EACH_NARG(...) SCHEMA_FOR_EACH_NARG_(__VA_ARGS__, SCHEMA_FOR_EACH_RSEQ_N())
#define SCHEMA_FOR_EACH_NARG_(...) SCHEMA_EXPAND(SCHEMA_FOR_EACH_ARG_N(__VA_ARGS__))
#define SCHEMA_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define SCHEMA_FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define SCHEMA_CONCATENATE(x, y) x##y
#define SCHEMA_MACRO_CONCAT(x, y) SCHEMA_CONCATENATE(x, y)

#define SCHEMA_VERSION(N, U, S)                                                                                        \
    template <> struct Engine::Details::SchemaVersion<S> {                                                             \
        using Versioning = U;                                                                                          \
        static size_t getVersion() {                                                                                   \
            return U::getVersion<S>();                                                                                 \
        }                                                                                                              \
    };                                                                                                                 \
    template <> struct Engine::Details::SchemaUnpacker<S> {                                                            \
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
        Engine::Details::SchemaVersioning<SCHEMA_UNPACK(__VA_ARGS__)>;                                                 \
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
        throw std::runtime_error(fmt::format("No such index defined as: '{}'", typeid(decltype(F)).name()));           \
    }                                                                                                                  \
    static void putIndexes(Engine::DatabaseOperations& db, const std::string_view& key, const Self& value) {           \
        SCHEMA_FOR_EACH_INDEX_PUT(__VA_ARGS__)                                                                         \
    }                                                                                                                  \
    static void removeIndexes(Engine::DatabaseOperations& db, const std::string_view& key, const Self& value) {        \
        SCHEMA_FOR_EACH_INDEX_REMOVE(__VA_ARGS__)                                                                      \
    }

#define SCHEMA_INDEXES_NONE()                                                                                          \
    static void putIndexes(Engine::DatabaseOperations& db, const std::string_view& key, const Self& value) {           \
        (void)db;                                                                                                      \
        (void)value;                                                                                                   \
        (void)key;                                                                                                     \
    }                                                                                                                  \
    static void removeIndexes(Engine::DatabaseOperations& db, const std::string_view& key, const Self& value) {        \
        (void)db;                                                                                                      \
        (void)value;                                                                                                   \
        (void)key;                                                                                                     \
    }
} // namespace Engine
