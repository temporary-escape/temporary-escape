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

template <typename T> struct SchemaGetVarClass {};

template <typename Class, typename Value> struct SchemaGetVarClass<Value Class::*> { using type = Class; };

template <typename M> struct SchemaGetVarType {
    template <typename C, typename T> static T getType(T C::*v) {
    }

    typedef decltype(getType(static_cast<M>(nullptr))) type;
};

#define SCHEMA_DEFINE(Class)                                                                                           \
    static const char* schemaGetName() {                                                                               \
        return SCHEMA_STRING(Class);                                                                                   \
    }                                                                                                                  \
    template <auto F> static const char* schamaGetFieldName();                                                         \
    void schemaInsertIndexes(AbstractDatabase& db, const std::string& key) const {                                     \
        (void)key;                                                                                                     \
        (void)db;                                                                                                      \
    }

#define SCHEMA_DEFINE_FIELD_NAME(Class, Field)                                                                         \
    template <> static const char* schamaGetFieldName<&Class::Field>() {                                               \
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

#define SCHEMA_PUT_INDEX_CALL(C, F) db.putIndex<&C::F>(key, this->F)
#define SCHEMA_FOR_EACH_INDEX_PUT(cls, ...) SCHEMA_FOR_EACH(SCHEMA_PUT_INDEX_CALL, cls, __VA_ARGS__)
#define SCHEMA_FOR_EACH_INDEX_DEFINE(cls, ...) SCHEMA_FOR_EACH(SCHEMA_DEFINE_FIELD_NAME, cls, __VA_ARGS__)

#define SCHEMA_DEFINE_INDEXED(cls, ...)                                                                                \
    static const char* schemaGetName() {                                                                               \
        return SCHEMA_STRING(cls);                                                                                     \
    }                                                                                                                  \
    template <auto F> static const char* schamaGetFieldName();                                                         \
    SCHEMA_FOR_EACH_INDEX_DEFINE(cls, __VA_ARGS__);                                                                    \
    void schemaInsertIndexes(AbstractDatabase& db, const std::string& key) const {                                     \
        SCHEMA_FOR_EACH_INDEX_PUT(cls, __VA_ARGS__);                                                                   \
    }

template <typename Class> struct SchemaHelper {
    static const char* getClassName() {
        return Class::schemaGetName();
    }

    static std::string getDataKeyName(const std::string& key) {
        return fmt::format("{}:data:{}", getClassName(), key);
    }

    template <typename FieldType, FieldType Class::*Field> static const char* getFieldName() {
        return Class::template schamaGetFieldName<Field>();
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
        value.schemaInsertIndexes(db, key);
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

/*template <typename T> struct IndexValueHelper {
    static std::string get(const T& value) {
        return fmt::format("{}", value);
    }
};

template <> struct IndexValueHelper<std::string> {
    static std::string get(const std::string& value) {
        return value;
    }
};

template <typename C, typename T, T C::*Field> struct IndexValueExtractor {
    static std::string get(const C& schema) {
        return IndexValueHelper<T>::get(schema.*Field);
    }
};

template <typename C> using IndexValueFunction = std::string (*)(const C&);

template <typename C> struct IndexMapping {
    const char* name;
    const IndexValueFunction<C> func;
};

template <typename C, typename T, T C::*Field> struct SchemaIndexName { static const char* getName(); };

template <typename T> struct SchemaIndexes {
    static void putIndexes(AbstractDatabase& db, const std::string& key, const T& value);
    static void removeIndexes(AbstractDatabase& db, const std::string& key);
};

template <typename T> struct SchemaNaming { static const char* getName(); };

#define SCHEMA_DEFINE_NAMED(T, N)                                                                                      \
    template <> struct SchemaNaming<T> {                                                                               \
        static const char* getName() {                                                                                 \
            return N;                                                                                                  \
        }                                                                                                              \
    };

#define SCHEMA_DEFINE(T) SCHEMA_DEFINE_NAMED(T, #T)

template <typename T> struct SchemaKeyName {
    static std::string getName(const std::string& key) {
        return fmt::format("{}:data:{}", SchemaNaming<T>::getName(), key);
    }
};

template <> struct SchemaKeyName<std::string> {
    static std::string getName(const std::string& key) {
        return key;
    }
};

template <typename M> struct SchemaGetVarType {
    template <typename C, typename T> static T getType(T C::*v) {
    }

    typedef decltype(getType(static_cast<M>(nullptr))) type;
};

template <typename T> struct SchemaGetVarClass {};

template <typename Class, typename Value> struct SchemaGetVarClass<Value Class::*> { using type = Class; };*/

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

    template <typename T> std::vector<T> seek(const std::string& prefix) {
        std::vector<T> values;

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

        seekInternal(SchemaHelper<T>::getDataKeyName(prefix), handler);

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
                              const std::function<void(const std::string&, const char*, size_t)>& fn) = 0;
    virtual bool getInternal(const std::string& key, const std::function<void(const char*, size_t)>& fn) = 0;
    virtual void multiGetInternal(const std::vector<std::string>& keys,
                                  const std::function<void(const std::string&, const char*, size_t)>& fn) = 0;
    virtual void putInternal(const std::string& key, const char* rawData, size_t size) = 0;
    virtual void removeInternal(const std::string& key) = 0;
    virtual void removeByPrefixInternal(const std::string& prefix) = 0;
};

/*template <typename T>
inline void SchemaIndexes<T>::putIndexes(AbstractDatabase& db, const std::string& key, const T& value) {
    // By default do nothing
    (void)db;
    (void)key;
    (void)value;
}

template <typename T> inline void SchemaIndexes<T>::removeIndexes(AbstractDatabase& db, const std::string& key) {
    // By default do nothing
    (void)db;
    (void)key;
}

template <typename T, size_t N>
inline void putIndexes(AbstractDatabase& db, const std::string& key, const T& value,
                       const std::array<IndexMapping<T>, N>& fields) {
    for (const auto& field : fields) {
        const auto indexKey =
            fmt::format("{}:index:{}:{}:{}", SchemaNaming<T>::getName(), field.name, field.func(value), key);
        db.put(indexKey, SchemaKeyName<T>::getName(key));
    }
}

template <typename T, size_t N>
inline void removeIndexes(AbstractDatabase& db, const std::string& key, const std::array<IndexMapping<T>, N>& fields) {
    // TODO (do we need it anyway?)
}*/

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
                      const std::function<void(const std::string&, const char*, size_t)>& fn) override;
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

/*#define SCHEMA_INDEX_NAME(C, F) \
    template <> struct SchemaIndexName<C, decltype(C::F), &C::F> {                                                     \
        static const char* getName() {                                                                                 \
            return #F;                                                                                                 \
        }                                                                                                              \
    };

#define SCHEMA_INDEX_MAPPING(C, F)                                                                                     \
    IndexMapping<C> {                                                                                                  \
#F, &IndexValueExtractor < C, decltype(C::F), &C::F> ::get                                                     \
    }

#define SCHEMA_INDEX_MAPPING_ARRAY(C, ...)                                                                             \
    static constexpr std::array<IndexMapping<C>, PP_NARG(__VA_ARGS__) / 4> fields = {__VA_ARGS__};

#define SCHEMA_MAPPING_1(C, a) SCHEMA_INDEX_MAPPING(C, a),
#define SCHEMA_MAPPING_2(C, a, b) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_1(C, b)
#define SCHEMA_MAPPING_3(C, a, b, c) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_2(C, b, c)
#define SCHEMA_MAPPING_4(C, a, b, c, d) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_3(C, b, c, d)
#define SCHEMA_MAPPING_5(C, a, b, c, d, e) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_4(C, b, c, d, e)
#define SCHEMA_MAPPING_6(C, a, b, c, d, e, f) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_5(C, b, c, d, e, f)
#define SCHEMA_MAPPING_7(C, a, b, c, d, e, f, g) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_6(C, b, c, d, e, f, g)
#define SCHEMA_MAPPING_8(C, a, b, c, d, e, f, g, h) SCHEMA_MAPPING_1(C, a) SCHEMA_MAPPING_7(C, b, c, d, e, f, g, h)
#define SCHEMA_MAPPING_M(C, M, ...) M(C, __VA_ARGS__)
#define SCHEMA_MAPPING_LIST(C, ...) SCHEMA_MAPPING_M(C, XPASTE(SCHEMA_MAPPING_, PP_NARG(__VA_ARGS__)), __VA_ARGS__)

#define SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_NAME(C, a)
#define SCHEMA_INDEX_2(C, a, b) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_1(C, b)
#define SCHEMA_INDEX_3(C, a, b, c) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_2(C, b, c)
#define SCHEMA_INDEX_4(C, a, b, c, d) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_3(C, b, c, d)
#define SCHEMA_INDEX_5(C, a, b, c, d, e) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_4(C, b, c, d, e)
#define SCHEMA_INDEX_6(C, a, b, c, d, e, f) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_5(C, b, c, d, e, f)
#define SCHEMA_INDEX_7(C, a, b, c, d, e, f, g) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_6(C, b, c, d, e, f, g)
#define SCHEMA_INDEX_8(C, a, b, c, d, e, f, g, h) SCHEMA_INDEX_1(C, a) SCHEMA_INDEX_7(C, b, c, d, e, f, g, h)
#define SCHEMA_INDEX_M(C, M, ...) M(C, __VA_ARGS__)
#define SCHEMA_INDEX_LIST(C, ...) SCHEMA_INDEX_M(C, XPASTE(SCHEMA_INDEX_, PP_NARG(##__VA_ARGS__)), __VA_ARGS__)

#define SCHEMA_DEFINE_INDEXED(C, ...)                                                                                  \
    SCHEMA_DEFINE(C)                                                                                                   \
    SCHEMA_INDEX_LIST(C, __VA_ARGS__)                                                                                  \
    template <> struct SchemaIndexes<C> {                                                                              \
        SCHEMA_INDEX_MAPPING_ARRAY(C, SCHEMA_MAPPING_LIST(C, __VA_ARGS__))                                             \
        static void putIndexes(AbstractDatabase& db, const std::string& key, const C& value) {                         \
            Scissio::putIndexes(db, key, value, fields);                                                               \
        }                                                                                                              \
        static void removeIndexes(AbstractDatabase& db, const std::string& key) {                                      \
            Scissio::removeIndexes(db, key, fields);                                                                   \
        }                                                                                                              \
    };
*/