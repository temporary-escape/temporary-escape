#pragma once

#include "../Utils/Exceptions.hpp"
#include "../Utils/Macros.hpp"
#include "../Utils/Msgpack.hpp"
#include "../Utils/Path.hpp"
#include <functional>
#include <optional>

#define SCHEMA_STRING(s) #s

namespace Scissio {
class AbstractDatabase;

template <typename T> struct SchemaDefinition {
    static const char* getName();

    static void insertIndexes(AbstractDatabase& db, const std::string& key, const T& o) {
        (void)db;
        (void)key;
        (void)o;
    }
};

template <typename Class, typename FieldType, FieldType Class::*Field> struct SchemaIndexDefinition {
    static const char* getName();
};

template <typename T> struct SchemaGetVarClass {};

template <typename Class, typename Value> struct SchemaGetVarClass<Value Class::*> { using type = Class; };

template <typename M> struct SchemaGetVarType {
    template <typename C, typename T> static T getType(T C::*v) {
    }

    typedef decltype(getType(static_cast<M>(nullptr))) type;
};

#define SCHEMA_DEFINE(Class)                                                                                           \
    template <> inline const char* SchemaDefinition<Class>::getName() {                                                \
        return SCHEMA_STRING(Class);                                                                                   \
    }

#define SCHEMA_DEFINE_FIELD_NAME(Class, Field)                                                                         \
    template <> inline const char* SchemaIndexDefinition<Class, decltype(Class::Field), &Class::Field>::getName() {    \
        return SCHEMA_STRING(Field);                                                                                   \
    }

// Source: https://stackoverflow.com/a/14735113
#define SCHEMA_FOR_EACH_EXPAND(x) x
#define SCHEMA_FOR_EACH_1(what, cls, x, ...) what(cls, x)
#define SCHEMA_FOR_EACH_2(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_1(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH_3(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_2(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH_4(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_3(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH_5(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_4(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH_6(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_5(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH_7(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_6(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH_8(what, cls, x, ...)                                                                           \
    what(cls, x);                                                                                                      \
    EXPAND(SCHEMA_FOR_EACH_7(what, cls, __VA_ARGS__))

#define SCHEMA_FOR_EACH_NARG(...) SCHEMA_FOR_EACH_NARG_(__VA_ARGS__, SCHEMA_FOR_EACH_RSEQ_N())
#define SCHEMA_FOR_EACH_NARG_(...) SCHEMA_FOR_EACH_EXPAND(SCHEMA_FOR_EACH_ARG_N(__VA_ARGS__))
#define SCHEMA_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define SCHEMA_FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
#define SCHEMA_FOR_EACH_CONCATENATE(x, y) x##y
#define SCHEMA_FOR_EACH_(N, what, cls, ...)                                                                            \
    SCHEMA_FOR_EACH_EXPAND(SCHEMA_FOR_EACH_CONCATENATE(SCHEMA_FOR_EACH_, N)(what, cls, __VA_ARGS__))
#define SCHEMA_FOR_EACH(what, cls, ...) SCHEMA_FOR_EACH_(SCHEMA_FOR_EACH_NARG(__VA_ARGS__), what, cls, __VA_ARGS__)

#define SCHEMA_PUT_INDEX_CALL(C, F) db.putIndex<&C::F>(key, o.F)
#define SCHEMA_FOR_EACH_INDEX_PUT(cls, ...) SCHEMA_FOR_EACH(SCHEMA_PUT_INDEX_CALL, cls, __VA_ARGS__)
#define SCHEMA_FOR_EACH_INDEX_DEFINE(cls, ...) SCHEMA_FOR_EACH(SCHEMA_DEFINE_FIELD_NAME, cls, __VA_ARGS__)

#define SCHEMA_DEFINE_INDEXED(cls, ...)                                                                                \
    template <> inline const char* SchemaDefinition<cls>::getName() {                                                  \
        return SCHEMA_STRING(cls);                                                                                     \
    }                                                                                                                  \
    SCHEMA_FOR_EACH_INDEX_DEFINE(cls, __VA_ARGS__);                                                                    \
    template <>                                                                                                        \
    inline void SchemaDefinition<cls>::insertIndexes(AbstractDatabase& db, const std::string& key, const cls& o) {     \
        SCHEMA_FOR_EACH_INDEX_PUT(cls, __VA_ARGS__);                                                                   \
    }

template <typename Class> struct SchemaHelper {
    static const char* getClassName() {
        return SchemaDefinition<Class>::getName();
    }

    static std::string getDataKeyName(const std::string& key) {
        return fmt::format("{}:data:{}", getClassName(), key);
    }

    template <typename FieldType, FieldType Class::*Field> static const char* getFieldName() {
        return SchemaIndexDefinition<Class, FieldType, Field>::getName();
    }

    template <typename FieldType, FieldType Class::*Field> static std::string getIndexKeyName() {
        return fmt::format("{}:index:{}:", getClassName(), getFieldName<FieldType, Field>());
    }

    template <typename FieldType, FieldType Class::*Field> static std::string getIndexKeyName(const FieldType& value) {
        return fmt::format("{}:index:{}:{}:", getClassName(), getFieldName<FieldType, Field>(), value);
    }

    template <typename FieldType, FieldType Class::*Field>
    static std::string getIndexKeyName(const FieldType& value, const std::string& key) {
        return fmt::format("{}:index:{}:{}:{}", getClassName(), getFieldName<FieldType, Field>(), value, key);
    }

    static void removeIndexes(AbstractDatabase& db, const std::string& key) {
        (void)db;
        (void)key;
        // Do we need this?
    }

    static void insertIndexes(AbstractDatabase& db, const std::string& key, const Class& value) {
        SchemaDefinition<Class>::insertIndexes(db, key, value);
    }
};

template <> struct SchemaHelper<std::string> {
    static const char* getClassName() {
        return "string";
    }

    static std::string getDataKeyName(const std::string& key) {
        return key;
    }

    static void insertIndexes(AbstractDatabase& db, const std::string& key, const std::string& value) {
        (void)db;
        (void)key;
        (void)value;
    }
};

class SCISSIO_API AbstractDatabase {
public:
    virtual ~AbstractDatabase() = default;

    template <typename T> void remove(const std::string& key) {
        removeInternal(SchemaHelper<T>::getDataKeyName(key));
        SchemaHelper<T>::removeIndexes(*this, key);
    }

    template <typename T> void removeByPrefix(const std::string& prefix) {
        removeByPrefixInternal(SchemaHelper<T>::getDataKeyName(prefix));
        SchemaHelper<T>::removeIndexes(*this, prefix);
    }

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

        if (getInternal(SchemaHelper<T>::getDataKeyName(key), handler)) {
            return value;
        }
        return std::nullopt;
    }

    template <typename T> std::vector<T> multiGet(const std::vector<std::string>& keys) {
        std::vector<std::string> keysInternal;
        keysInternal.reserve(keys.size());
        for (const auto& key : keys) {
            keysInternal.push_back(SchemaHelper<T>::getDataKeyName(key));
        }

        std::vector<T> values;
        values.reserve(keys.size());

        const auto handler = [&](const std::string& key, const char* data, const size_t size) {
            try {
                values.emplace_back();
                msgpack::unpacked result;
                msgpack::unpack(result, data, size);
                msgpack::object obj(result.get());
                obj.convert(values.back());
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: {} as: {}", key, typeid(T).name());
            }
        };

        multiGetInternal(keysInternal, handler);
        return values;
    }

    template <typename T> std::vector<T> seek(const std::string& prefix, const size_t max = 0) {
        std::vector<T> values;

        const auto handler = [&](const std::string& key, const char* data, const size_t size) -> bool {
            try {
                values.emplace_back();
                msgpack::unpacked result;
                msgpack::unpack(result, data, size);
                msgpack::object obj(result.get());
                obj.convert(values.back());
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: {} as: {}", key, typeid(T).name());
            }

            return max == 0 || values.size() < max;
        };

        seekInternal(SchemaHelper<T>::getDataKeyName(prefix), handler);

        return values;
    }

    template <typename T>
    std::vector<T> next(const std::string& prefix, const std::string& start, const size_t max = 0,
                        std::string* lastKey = nullptr) {
        std::vector<T> values;
        const auto substrLength = SchemaHelper<T>::getDataKeyName("").size();
        const auto keyPrefix = SchemaHelper<T>::getDataKeyName(prefix);
        const auto keyStart = SchemaHelper<T>::getDataKeyName(start);

        const auto handler = [&](const std::string& key, const char* data, const size_t size) -> bool {
            if (key == keyStart || key.find(keyPrefix) != 0) {
                return true;
            }

            try {
                values.emplace_back();
                msgpack::unpacked result;
                msgpack::unpack(result, data, size);
                msgpack::object obj(result.get());
                obj.convert(values.back());
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: {} as: {}", key, typeid(T).name());
            }

            if (lastKey) {
                *lastKey = key.substr(substrLength);
            }
            return max == 0 || values.size() < max;
        };

        nextInternal(keyPrefix, keyStart, handler);

        return values;
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    std::vector<T> getByIndex(const R& value) {
        return getByIndexInternal<T, R, F>(value);
    }

    template <typename T> void put(const std::string& key, const T& value) {
        msgpack::sbuffer sbuf;
        try {
            msgpack::packer<msgpack::sbuffer> packer(sbuf);
            msgpack::pack(sbuf, value);
        } catch (...) {
            EXCEPTION_NESTED("Failed to pack database key: {} as: {}", key, typeid(T).name());
        }

        putInternal(SchemaHelper<T>::getDataKeyName(key), sbuf.data(), sbuf.size());
        SchemaHelper<T>::insertIndexes(*this, key, value);
    }

    template <auto F, typename T = typename SchemaGetVarClass<decltype(F)>::type,
              typename R = typename SchemaGetVarType<decltype(F)>::type>
    void putIndex(const std::string& key, const R& value) {
        return putIndexInternal<T, R, F>(key, value);
    }

    template <typename... Fields> void putIndexes(const std::string& key, Fields&&... fields) {
        (putIndex(key, fields), ...);
    }

private:
    template <typename T, typename D, D T::*Field> void putIndexInternal(const std::string& key, const D& value) {
        put(SchemaHelper<T>::template getIndexKeyName<D, Field>(value, key), SchemaHelper<T>::getDataKeyName(key));
    }

    template <typename T, typename D, D T::*Field> std::vector<T> getByIndexInternal(const D& value) {
        const auto keys = seek<std::string>(SchemaHelper<T>::template getIndexKeyName<D, Field>(value));
        if (keys.empty()) {
            return {};
        }

        std::vector<T> values;
        values.reserve(keys.size());

        const auto handler = [&](const std::string& key, const char* data, const size_t size) {
            try {
                values.emplace_back();
                msgpack::unpacked result;
                msgpack::unpack(result, data, size);
                msgpack::object obj(result.get());
                obj.convert(values.back());
            } catch (...) {
                EXCEPTION_NESTED("Failed to unpack database key: {} as: {}", key, typeid(T).name());
            }
        };

        multiGetInternal(keys, handler);
        return values;
    }

    virtual void seekInternal(const std::string& prefix,
                              const std::function<bool(const std::string&, const char*, size_t)>& fn) = 0;
    virtual void nextInternal(const std::string& key, const std::string& start,
                              const std::function<bool(const std::string&, const char*, size_t)>& fn) = 0;
    virtual bool getInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) = 0;
    virtual void multiGetInternal(const std::vector<std::string>& keys,
                                  const std::function<void(const std::string&, const char*, size_t)>& fn) = 0;
    virtual void putInternal(const std::string& key, const char* rawData, size_t size) = 0;
    virtual void removeInternal(const std::string& key) = 0;
    virtual void removeByPrefixInternal(const std::string& prefix) = 0;
};

class SCISSIO_API Transaction : public AbstractDatabase {
public:
    virtual ~Transaction() = default;

    template <typename T> std::optional<T> getForUpdate(const std::string& key) {
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

        if (getForUpdateInternal(SchemaHelper<T>::getDataKeyName(key), handler)) {
            return value;
        }
        return std::nullopt;
    }

private:
    virtual bool getForUpdateInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) = 0;
};

class SCISSIO_API Database : public AbstractDatabase {
public:
    explicit Database(const Path& path);
    ~Database() override;

    bool transaction(const std::function<bool(Transaction&)>& fn, bool retry = true);

    template <typename T> bool update(const std::string& key, const std::function<bool(std::optional<T>&)>& fn) {
        return transaction([&](Transaction& txn) -> bool {
            auto found = txn.template getForUpdate<T>(key);

            const auto res = fn(found);
            if (res && found.has_value()) {
                txn.template put<T>(key, found.value());
                return true;
            }

            return false;
        });
    }

private:
    void seekInternal(const std::string& prefix,
                      const std::function<bool(const std::string&, const char*, size_t)>& fn) override;
    void nextInternal(const std::string& key, const std::string& start,
                      const std::function<bool(const std::string&, const char*, size_t)>& fn) override;
    bool getInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) override;
    void multiGetInternal(const std::vector<std::string>& keys,
                          const std::function<void(const std::string&, const char*, size_t)>& fn) override;
    void putInternal(const std::string& key, const char* rawData, size_t size) override;
    void removeInternal(const std::string& key) override;
    void removeByPrefixInternal(const std::string& key) override;

    struct Data;

    std::unique_ptr<Data> data;
};
} // namespace Scissio
