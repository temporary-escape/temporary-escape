#pragma once

#include "../Library.hpp"
#include "../Utils/Exceptions.hpp"
#include "../Utils/Macros.hpp"
#include "../Utils/MsgpackAdaptors.hpp"
#include <msgpack.hpp>

namespace Engine {
class Database;

namespace Details {
template <typename T> struct SchemaDefinition;

template <typename T> struct SchemaIndexes {
    template <auto F> static const char* getIndexName() {
        EXCEPTION("Not a valid index: {} for schema: '{}'", typeid(decltype(F)).name());
    }
    static bool hasIndexName(const std::string_view& index) {
        (void)index;
        return false;
    }
    static bool hasIndexes() {
        return false;
    }
    static void putIndexes(Database& db, const std::string_view& key, const T& value) {
        (void)db;
        (void)key;
        (void)value;
    }
    static void removeIndexes(Database& db, const std::string_view& key, const T& value) {
        (void)db;
        (void)key;
        (void)value;
    }
};

template <typename T> struct SchemaUnpacker {
    static void unpack(const msgpack::object& obj, T& value, const size_t version) {
        if (version != 1ULL) {
            EXCEPTION("Unable to unpack schema: '{}' with version: '{}' error: Only version 1 defined as default",
                      SchemaDefinition<T>::getName(),
                      version);
        }
        obj.convert(value);
    }
};

template <typename T> struct SchemaGetVarClass {};

template <typename Class, typename Value> struct SchemaGetVarClass<Value Class::*> {
    using type = Class;
};

template <typename M> struct SchemaGetVarType {
    template <typename C, typename T> static T getType(T C::*v) {
        (void)v;
    }

    typedef decltype(getType(static_cast<M>(nullptr))) type;
};

template <typename T> static std::string keyToSchemaDataKey(const std::string_view& key) {
    return fmt::format("{}:data:{}", SchemaDefinition<T>::getName(), key);
}

template <typename T, auto F> static const char* getSchemaIndexName() {
    return SchemaIndexes<T>::template getIndexName<F>();
}

template <typename T> static bool hasSchemaIndexName(const std::string_view& index) {
    return SchemaIndexes<T>::hasIndexName(index);
}

template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
          typename R = typename SchemaGetVarType<decltype(F)>::type>
static std::string keyToSchemaIndexKey() {
    return fmt::format("{}:index:{}:", SchemaDefinition<T>::getName(), getSchemaIndexName<T, F>());
}

template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
          typename R = typename SchemaGetVarType<decltype(F)>::type>
static std::string keyToSchemaIndexKey(const R& value) {
    return fmt::format("{}:index:{}:{}:", SchemaDefinition<T>::getName(), getSchemaIndexName<T, F>(), value);
}

template <typename T, typename V>
static std::string keyToSchemaIndexNameKey(const std::string_view& index, const V& value) {
    return fmt::format("{}:index:{}:{}:", SchemaDefinition<T>::getName(), index, value);
}

template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
          typename R = typename SchemaGetVarType<decltype(F)>::type>
static std::string keyToSchemaIndexKey(const R& value, const std::string_view& key) {
    return fmt::format("{}:index:{}:{}:{}", SchemaDefinition<T>::getName(), getSchemaIndexName<T, F>(), value, key);
}

template <typename T> msgpack::sbuffer packSchema(const T& value);
template <typename T> void unpackSchema(const msgpack::object& obj, T& value);
} // namespace Details

class ENGINE_API Database {
public:
    class Transaction;
    class ObjectIterator;
    template <typename T> class Iterator;

    using TransactionCallback = std::function<bool(Database&)>;

    virtual ~Database() = default;

    // Low level operations
    virtual std::optional<msgpack::object_handle> getRaw(const std::string_view& key) = 0;
    virtual std::vector<msgpack::object_handle> multiGetRaw(const std::vector<std::string>& keys) = 0;
    virtual void putRaw(const std::string_view& key, const void* data, size_t size) = 0;
    virtual std::unique_ptr<Transaction> startTransaction() = 0;
    virtual void removeRaw(const std::string_view& key) = 0;
    virtual std::unique_ptr<ObjectIterator> seekRaw(const std::string_view& prefix,
                                                    const std::optional<std::string_view>& lowerBound) = 0;

    inline bool transaction(const TransactionCallback& callback);
    template <typename T> std::optional<T> find(const std::string_view& key);
    template <typename T> T get(const std::string_view& key);
    template <typename T> void put(const std::string_view& key, const T& value);
    template <typename T> std::vector<T> multiGet(const std::vector<std::string>& keys);
    template <typename T> void remove(const std::string_view& key);
    template <typename T> void removeByPrefix(const std::string_view& key);
    template <typename T>
    Iterator<T> seek(const std::string_view& prefix, const std::optional<std::string_view>& lowerBound = std::nullopt);
    template <typename T>
    std::vector<T> next(const std::string_view& prefix, const std::string_view& start, size_t max,
                        std::string* lastKey);
    template <typename T> std::vector<T> seekAll(const std::string& key, size_t max = 0);
    template <typename T> T update(const std::string_view& key, const std::function<T(std::optional<T>)>& callback);
    template <auto F, typename T = typename Details::SchemaGetVarClass<decltype(F)>::type,
              typename R = typename Details::SchemaGetVarType<decltype(F)>::type>
    std::vector<T> getByIndex(const R& value);
    template <typename T, typename V> std::vector<T> getByIndexName(const std::string_view& index, const V& value);

    template <auto F, typename T = typename Details::SchemaGetVarClass<decltype(F)>::type,
              typename R = typename Details::SchemaGetVarType<decltype(F)>::type>
    void putIndex(const std::string_view& key, const R& value);

    template <auto F, typename T = typename Details::SchemaGetVarClass<decltype(F)>::type,
              typename R = typename Details::SchemaGetVarType<decltype(F)>::type>
    void removeIndex(const std::string_view& key, const R& value);
};

class Database::Transaction : public Database {
public:
    virtual ~Transaction() = default;

    virtual bool commit() = 0;
    virtual void abort() = 0;
};

class Database::ObjectIterator {
public:
    virtual ~ObjectIterator() = default;

    virtual bool next() = 0;
    virtual void close() = 0;
    virtual operator bool() const = 0;
    [[nodiscard]] virtual const std::string& key() const = 0;
    [[nodiscard]] virtual const msgpack::object_handle& value() const = 0;
    [[nodiscard]] virtual std::string valueString() const = 0;
};

template <typename T> class Database::Iterator {
public:
    explicit Iterator(std::unique_ptr<ObjectIterator> iter) : iter{std::move(iter)} {
    }
    virtual ~Iterator() = default;
    Iterator(const Iterator& other) = delete;
    Iterator(Iterator&& other) noexcept = default;
    Iterator& operator=(const Iterator& other) = delete;
    Iterator& operator=(Iterator&& other) noexcept = default;

    bool next() {
        return iter->next();
    }
    void close() {
        iter->close();
    }
    operator bool() const {
        return *iter;
    }
    [[nodiscard]] const std::string& key() const {
        return iter->key();
    }
    [[nodiscard]] T value() const {
        T res{};
        Details::unpackSchema(iter->value().get(), res);
        return res;
    }
    [[nodiscard]] std::string valueString() const {
        return iter->valueString();
    }

private:
    std::unique_ptr<ObjectIterator> iter;
};

template <typename T> std::optional<T> Database::find(const std::string_view& key) {
    try {
        const auto fullKey = Details::keyToSchemaDataKey<T>(key);
        const auto object = getRaw(fullKey);

        if (object) {
            T value{};
            Details::unpackSchema(object->get(), value);
            return value;
        }

        return {};
    } catch (std::exception& e) {
        EXCEPTION("Failed to get database key: {} schema: {} error: {}",
                  key,
                  Details::SchemaDefinition<T>::getName(),
                  e.what());
    }
}

template <typename T> T Database::get(const std::string_view& key) {
    auto item = this->template find<T>(key);
    if (!item) {
        EXCEPTION("No such database key: {} schema: {}", key, Details::SchemaDefinition<T>::getName());
    }
    return std::move(item.value());
}

template <typename T> void Database::put(const std::string_view& key, const T& value) {
    try {
        const auto sbuf = Details::packSchema<T>(value);
        const auto fullKey = Details::keyToSchemaDataKey<T>(key);

        putRaw(fullKey, sbuf.data(), sbuf.size());
        Details::SchemaIndexes<T>::putIndexes(*this, key, value);

    } catch (std::exception& e) {
        EXCEPTION("Failed to put database key: {} schema: {} error: {}",
                  key,
                  Details::SchemaDefinition<T>::getName(),
                  e.what());
    }
}

template <typename T> std::vector<T> Database::multiGet(const std::vector<std::string>& keys) {
    try {
        std::vector<T> values;

        std::vector<std::string> keysInternal;
        keysInternal.reserve(keys.size());
        values.reserve(keys.size());

        for (const auto& key : keys) {
            keysInternal.push_back(Details::keyToSchemaDataKey<T>(key));
        }

        const auto objects = multiGetRaw(keysInternal);

        for (const auto& object : objects) {
            if (object->type == msgpack::type::NIL) {
                continue;
            }
            values.emplace_back();
            Details::unpackSchema(object.get(), values.back());
        }

        return values;
    } catch (std::exception& e) {
        EXCEPTION(
            "Failed to multi-get database schema: {} error: {}", Details::SchemaDefinition<T>::getName(), e.what());
    }
}

template <typename T> void Database::remove(const std::string_view& key) {
    const auto fullKey = Details::keyToSchemaDataKey<T>(key);
    try {
        if (Details::SchemaIndexes<T>::hasIndexes()) {
            const auto found = find<T>(key);
            if (found.has_value()) {
                Details::SchemaIndexes<T>::removeIndexes(*this, key, found.value());
            }
        }
        removeRaw(fullKey);
    } catch (std::exception& e) {
        EXCEPTION("Failed to delete database key: {} schema: {} error: {}",
                  key,
                  Details::SchemaDefinition<T>::getName(),
                  e.what());
    }
}

template <typename T> void Database::removeByPrefix(const std::string_view& key) {
    const auto fullKey = Details::keyToSchemaDataKey<T>(key);
    try {
        auto it = this->seekRaw(fullKey, std::nullopt);
        while (it->next()) {
            if (Details::SchemaIndexes<T>::hasIndexes()) {
                T value{};
                Details::unpackSchema(it->value().get(), value);
                Details::SchemaIndexes<T>::removeIndexes(*this, key, value);
            }
            removeRaw(it->key());
        }
    } catch (std::exception& e) {
        EXCEPTION("Failed to delete database key by prefix: {} schema: {} error: {}",
                  key,
                  Details::SchemaDefinition<T>::getName(),
                  e.what());
    }
}

template <typename T>
Database::Iterator<T> Database::seek(const std::string_view& prefix,
                                     const std::optional<std::string_view>& lowerBound) {
    const auto fullKey = Details::keyToSchemaDataKey<T>(prefix);
    std::string fullLowerBound;
    if (lowerBound.has_value()) {
        fullLowerBound = Details::keyToSchemaDataKey<T>(lowerBound.value());
    }
    return Iterator<T>{seekRaw(fullKey, fullLowerBound)};
}

template <typename T> std::vector<T> Database::seekAll(const std::string& key, const size_t max) {
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
std::vector<T> Database::next(const std::string_view& prefix, const std::string_view& start, const size_t max,
                              std::string* lastKey) {
    std::vector<T> items;
    const auto substrLength = Details::SchemaDefinition<T>::getName().size() + 6; // + strlen(":data:")
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

template <typename T>
T Database::update(const std::string_view& key, const std::function<T(std::optional<T>)>& callback) {
    std::optional<T> found;
    transaction([&](Database& txn) -> bool {
        found = txn.template find<T>(key);
        found = callback(std::move(found));
        txn.template put<T>(key, found.value());
        return true;
    });

    if (!found) {
        EXCEPTION("Failed to update database key: {} schema: {} error: callback did not produce any value",
                  key,
                  Details::SchemaDefinition<T>::getName());
    }

    return found.value();
}

template <auto F, typename T, typename R> std::vector<T> Database::getByIndex(const R& value) {
    try {
        const auto fullIndexKey = Details::keyToSchemaIndexKey<F>(value);

        std::vector<T> items;

        auto it = seekRaw(fullIndexKey, std::nullopt);
        while (it->next()) {
            const auto key = it->valueString();
            const auto item = getRaw(key);

            if (item) {
                items.emplace_back();
                Details::unpackSchema(item->get(), items.back());
            }
        }

        return items;
    } catch (std::exception& e) {
        EXCEPTION("Failed to delete database values by index: {} value: {} schema: {} error: {}",
                  Details::SchemaIndexes<T>::template getIndexName<F>(),
                  value,
                  Details::SchemaDefinition<T>::getName(),
                  e.what());
    }
}

template <typename T, typename V>
std::vector<T> Database::getByIndexName(const std::string_view& index, const V& value) {
    try {
        const auto fullIndexKey = Details::keyToSchemaIndexNameKey<T, V>(value);

        std::vector<T> items;

        auto it = seekRaw(fullIndexKey, std::nullopt);
        while (it->next()) {
            const auto key = it->valueString();
            const auto item = getRaw(key);

            if (item) {
                items.emplace_back();
                Details::unpackSchema(item->get(), items.back());
            }
        }

        return items;
    } catch (std::exception& e) {
        EXCEPTION("Failed to delete database values by index: {} value: {} schema: {} error: {}",
                  index,
                  value,
                  Details::SchemaDefinition<T>::getName(),
                  e.what());
    }
}

template <auto F, typename T, typename R> void Database::putIndex(const std::string_view& key, const R& value) {
    const auto fullIndexKey = Details::keyToSchemaIndexKey<F>(value, key);
    const auto fullKey = Details::keyToSchemaDataKey<T>(key);
    putRaw(fullIndexKey, fullKey.data(), fullKey.size());
}

template <auto F, typename T, typename R> void Database::removeIndex(const std::string_view& key, const R& value) {
    const auto fullIndexKey = Details::keyToSchemaIndexKey<F>(value, key);
    removeRaw(fullIndexKey);
}

template <typename T> msgpack::sbuffer Details::packSchema(const T& value) {
    try {
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_array(2);
        packer.pack_uint64(SchemaDefinition<T>::getVersion());
        packer.pack(value);
        return sbuf;
    } catch (std::exception& e) {
        EXCEPTION("Failed to pack schema: {} error: {}", Details::SchemaDefinition<T>::getName(), e.what());
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

inline bool Database::transaction(const Database::TransactionCallback& callback) {
    while (true) {
        auto txn = startTransaction();
        if (callback(*txn)) {
            if (!txn->commit()) {
                continue;
            }
            return true;
        } else {
            txn->abort();
            return false;
        }
    }
}
} // namespace Engine

#define SCHEMA_DEFINE(Name)                                                                                            \
    template <> struct Details::SchemaDefinition<Name> {                                                               \
        static const std::string_view& getName() {                                                                     \
            static std::string_view name{#Name};                                                                       \
            return name;                                                                                               \
        }                                                                                                              \
        static uint64_t getVersion() {                                                                                 \
            return 1ULL;                                                                                               \
        }                                                                                                              \
    }

#define SCHEMA_IF_INDEX_NAME_FIELD(Field)                                                                              \
    if (reinterpret_cast<int T::*>(F) == reinterpret_cast<int T::*>(&T::Field)) {                                      \
        return #Field;                                                                                                 \
    }

#define SCHEMA_IF_INDEX_NAME_HAS(Field)                                                                                \
    if (index == #Field) {                                                                                             \
        return true;                                                                                                   \
    }

#define SCHEMA_INDEX_PUT_FIELD(Field) db.putIndex<&T::Field>(key, value.Field);

#define SCHEMA_INDEX_REMOVE_FIELD(Field) db.removeIndex<&T::Field>(key, value.Field);

#define SCHEMA_INDEXES(Name, ...)                                                                                      \
    template <> struct Details::SchemaIndexes<Name> {                                                                  \
        using T = Name;                                                                                                \
        template <auto F> static const char* getIndexName() {                                                          \
            FOR_EACH(SCHEMA_IF_INDEX_NAME_FIELD, __VA_ARGS__)                                                          \
                                                                                                                       \
            EXCEPTION("Not a valid index: {} for schema: '{}'", typeid(decltype(F)).name());                           \
        }                                                                                                              \
        static bool hasIndexName(const std::string_view& index) {                                                      \
            FOR_EACH(SCHEMA_IF_INDEX_NAME_HAS, __VA_ARGS__)                                                            \
            return false;                                                                                              \
        }                                                                                                              \
        static bool hasIndexes() {                                                                                     \
            return true;                                                                                               \
        }                                                                                                              \
        static void putIndexes(Engine::Database& db, const std::string_view& key, const T& value) {                    \
            FOR_EACH(SCHEMA_INDEX_PUT_FIELD, __VA_ARGS__)                                                              \
        }                                                                                                              \
        static void removeIndexes(Engine::Database& db, const std::string_view& key, const T& value) {                 \
            FOR_EACH(SCHEMA_INDEX_REMOVE_FIELD, __VA_ARGS__)                                                           \
        }                                                                                                              \
    }
