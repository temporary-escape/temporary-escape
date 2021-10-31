#pragma once
#include "../Config.hpp"
#include "../Library.hpp"
#include "../Math/Vector.hpp"
#include "../Utils/Path.hpp"
#include "Msgpack.hpp"
#include "StringUtils.hpp"

#include <array>
#include <cassert>
#include <fmt/format.h>
#include <functional>
#include <mutex>
#include <optional>
#include <sstream>

struct sqlite3;
struct sqlite3_stmt;

namespace Scissio {

class SCISSIO_API Database {
public:
    struct SCISSIO_API Stmt {
        static bool isNull(sqlite3_stmt* stmt, int idx);

        static void setInt64(sqlite3_stmt* stmt, int idx, int64_t value);
        static void setIntU64(sqlite3_stmt* stmt, int idx, uint64_t value);
        static void setInt32(sqlite3_stmt* stmt, int idx, int32_t value);
        static void setIntU32(sqlite3_stmt* stmt, int idx, uint32_t value);
        static void setFloat(sqlite3_stmt* stmt, int idx, float value);
        static void setDouble(sqlite3_stmt* stmt, int idx, double value);
        static void setBool(sqlite3_stmt* stmt, int idx, bool value);
        static void setString(sqlite3_stmt* stmt, int idx, const std::string& value);
        static void setConstChar(sqlite3_stmt* stmt, int idx, const char* value);
        static void setNull(sqlite3_stmt* stmt, int idx);
        static void setBlob(sqlite3_stmt* stmt, int idx, void* data, size_t length);

        static int64_t getInt64(sqlite3_stmt* stmt, int idx);
        static uint64_t getIntU64(sqlite3_stmt* stmt, int idx);
        static int32_t getInt32(sqlite3_stmt* stmt, int idx);
        static uint32_t getIntU32(sqlite3_stmt* stmt, int idx);
        static float getFloat(sqlite3_stmt* stmt, int idx);
        static double getDouble(sqlite3_stmt* stmt, int idx);
        static bool getBool(sqlite3_stmt* stmt, int idx);
        static std::string getString(sqlite3_stmt* stmt, int idx);
        static void getBlob(sqlite3_stmt* stmt, int idx, void* dst, size_t length);
    };

    template <typename T> struct Helper {
        static void set(sqlite3_stmt* stmt, int idx, const T& value);
        static T get(sqlite3_stmt* stmt, int idx);
    };

    struct Flags {
        static constexpr int Integer = 1 << 0;
        // static constexpr int Boolean = 1 << 1;
        static constexpr int Real = 1 << 2;
        static constexpr int Text = 1 << 3;
        static constexpr int Binary = 1 << 4;
        static constexpr int Primary = 1 << 5;
        static constexpr int AutoIncrement = 1 << 6;
        static constexpr int NonNull = 1 << 7;
        static constexpr int Foreign = 1 << 8;
        static constexpr int Indexed = 1 << 9;
        static constexpr int Unique = 1 << 10;
        static constexpr int OnDeleteNull = 1 << 11;
        static constexpr int OnDeleteDefault = 1 << 12;
        static constexpr int OnDeleteRestrict = 1 << 13;
        static constexpr int OnDeleteCascade = 1 << 14;
        static constexpr int OnUpdateNull = 1 << 15;
        static constexpr int OnUpdateDefault = 1 << 16;
        static constexpr int OnUpdateRestrict = 1 << 17;
        static constexpr int OnUpdateCascade = 1 << 18;
    };

    template <typename A, typename B> struct InnerJoin {
        A left;
        B right;
    };

    class Schema {
    public:
        class Field {
        public:
            std::string name;
            int flags{0};
            std::string def;
            std::string ref;

            explicit Field(std::string name) : name(std::move(name)) {
            }

            bool operator==(const Field& other) const {
                return name == other.name && flags == other.flags && def == other.def && ref == other.ref;
            }

            bool operator!=(const Field& other) const {
                return name != other.name || flags != other.flags || def != other.def || ref != other.ref;
            }

            Field& integer() {
                flags |= Flags::Integer;
                return *this;
            }

            Field& boolean() {
                flags |= Flags::Integer;
                return *this;
            }

            Field& real() {
                flags |= Flags::Real;
                return *this;
            }

            Field& text() {
                flags |= Flags::Text;
                return *this;
            }

            Field& binary() {
                flags |= Flags::Binary;
                return *this;
            }

            Field& primary() {
                flags |= Flags::Primary;
                return *this;
            }

            Field& autoInc() {
                flags |= Flags::AutoIncrement;
                return *this;
            }

            Field& nonNull() {
                flags |= Flags::NonNull;
                return *this;
            }

            Field& unique() {
                flags |= Flags::Unique;
                flags |= Flags::Indexed;
                return *this;
            }

            Field& indexed() {
                flags |= Flags::Indexed;
                return *this;
            }

            Field& onDeleteNull() {
                flags |= Flags::OnDeleteNull;
                return *this;
            }

            Field& onDeleteDefault() {
                flags |= Flags::OnDeleteDefault;
                return *this;
            }

            Field& onDeleteRestrict() {
                flags |= Flags::OnDeleteRestrict;
                return *this;
            }

            Field& onDeleteCascade() {
                flags |= Flags::OnDeleteCascade;
                return *this;
            }

            Field& onUpdateNull() {
                flags |= Flags::OnUpdateNull;
                return *this;
            }

            Field& onUpdateDefault() {
                flags |= Flags::OnUpdateDefault;
                return *this;
            }

            Field& onUpdateRestrict() {
                flags |= Flags::OnUpdateRestrict;
                return *this;
            }

            Field& onUpdateCascade() {
                flags |= Flags::OnUpdateCascade;
                return *this;
            }

            template <typename T> Field& references(const std::string& name) {
                ref = fmt::format("{} ({})", T::dbName(), name);
                flags |= Flags::Foreign;
                return *this;
            }

            Field& defval(std::string value) {
                this->def = std::move(value);
                return *this;
            }
        };

        /*class Index {
        public:
            explicit Index(const std::string& field) : name{field + "_idx"}, fields{field}, uniq{false} {
            }

            explicit Index(std::vector<std::string> fields)
                : name{Scissio::join("_", fields) + "_idx"}, fields{std::move(fields)}, uniq{false} {
            }

            Index& unique() {
                uniq = true;
                return *this;
            }

            bool operator==(const Index& other) const {
                return fields == other.fields && uniq == other.uniq;
            }

            bool operator!=(const Index& other) const {
                return fields != other.fields || uniq != other.uniq;
            }

        private:
            std::string name;
            std::vector<std::string> fields;
            bool uniq;
        };*/

        std::string name;
        std::vector<Field> fields;
        // std::vector<Index> indexes;

        bool operator==(const Schema& other) const {
            if (fields.size() != other.fields.size()) {
                return false;
            }

            /*if (indexes.size() != other.indexes.size()) {
                return false;
            }*/

            if (name != other.name) {
                return false;
            }

            for (size_t i = 0; i < fields.size(); i++) {
                if (fields.at(i) != other.fields.at(i)) {
                    return false;
                }
            }

            /*for (size_t i = 0; i < indexes.size(); i++) {
                if (indexes.at(i) != other.indexes.at(i)) {
                    return false;
                }
            }*/

            return true;
        }

        bool operator!=(const Schema& other) const {
            return !(*this == other);
        }
    };

    class SCISSIO_API Binder {
    public:
        explicit Binder(sqlite3_stmt* stmt);
        ~Binder();

        template <typename T> void bind(int idx, const T& value) {
            Helper<T>::set(stmt, idx, value);
        }

        template <typename T> T column(int idx) {
            return Helper<T>::get(stmt, idx);
        }

        void bindArgs(const int idx) {
            (void)idx;
        }

        template <typename Arg> void bindArgs(const int idx, const Arg& arg) {
            Helper<Arg>::set(stmt, idx, arg);
        }

        template <typename Arg, typename... Args> void bindArgs(const int idx, const Arg& arg, const Args&... args) {
            Helper<Arg>::set(stmt, idx, arg);
            bindArgs<Args...>(idx + 1, args...);
        }

    private:
        sqlite3_stmt* stmt;
    };

    using BinderCallback = std::function<void(Binder&, int)>;
    using SelectCallback = std::function<void(Binder&, int)>;

    explicit Database(const Path& path);
    explicit Database();
    Database(const Database& other) = delete;
    Database(Database&& other);
    virtual ~Database();
    Database& operator=(const Database& other) = delete;
    Database& operator=(Database&& other);
    void swap(Database& other);

    template <typename T> void create() {
        createOrMigrate(T::dbSchema());
    }

    template <typename T> void pragma(const std::string& name, const T& value) {
        const auto sql = fmt::format("PRAGMA {} = {};", name, value);
        query(sql, nullptr, nullptr);
    }

    template <typename T> T pragma(const std::string& name) {
        const auto sql = fmt::format("PRAGMA {};", name);
        T value{};
        query(sql, nullptr, [&](Binder& binder, const int idx) { value = binder.column<T>(idx); });
        return value;
    }

    template <typename T, typename V> std::optional<T> get(const std::string& field, const V& value) {
        const auto results = select<T>(fmt::format("WHERE {} = ?", field), value);
        if (results.empty()) {
            return std::nullopt;
        }
        return results.at(0);
    }

    template <typename T, typename V> std::optional<T> get(const V& id) {
        return get<T>("id", id);
    }

    template <typename T> uint64_t insert(T& value, bool ignore = false) {
        if (value.id != 0) {
            const auto sql = fmt::format("INSERT {}INTO {} (id,{}) VALUES (?,{});", ignore ? "OR IGNORE " : "",
                                         T::dbName(), T::dbNames(), T::dbValues());
            BinderCallback bind = [&](Binder& binder, int idx) {
                binder.bind(idx, value.id);
                value.dbBind(binder, idx + 1);
            };
            query(sql, bind, nullptr);
            return value.id;
        } else {
            // We need to acquire lock because getLastInsertRowId() is not thread safe
            std::lock_guard<std::mutex> lock{mutex};

            const auto sql = fmt::format("INSERT INTO {} ({}) VALUES ({});", T::dbName(), T::dbNames(), T::dbValues());
            BinderCallback bind = std::bind(&T::dbBind, &value, std::placeholders::_1, std::placeholders::_2);
            query(sql, bind, nullptr);
            value.id = getLastInsertRowId();
            return value.id;
        }
    }

    template <typename T, typename... Args> std::vector<T> select(const std::string& where = "", Args&&... args) {
        const auto sql = fmt::format("SELECT id,{} FROM {} {};", T::dbNames(), T::dbName(), where);
        std::vector<T> results;

        const auto bind = [&](Binder& binder, const int idx) { binder.bindArgs(idx, args...); };

        query(sql, bind, [&](Binder& binder, const int idx) {
            results.emplace_back();
            results.back().dbUnbind(binder, idx);
        });
        return results;
    }

    template <typename A, typename B, typename... Args>
    std::vector<InnerJoin<A, B>> join(const std::string& onA, const std::string& onB, const std::string& where = "",
                                      Args&&... args) {
        const auto sql = fmt::format("SELECT {}.id,{},{}.id,{} FROM {} INNER JOIN {} ON {}.{} = {}.{} {};", A::dbName(),
                                     A::dbSelect(), B::dbName(), B::dbSelect(), A::dbName(), B::dbName(), A::dbName(),
                                     onA, B::dbName(), onB, where);
        std::vector<InnerJoin<A, B>> results;

        const auto bind = [&](Binder& binder, const int idx) { binder.bindArgs(idx, args...); };

        query(sql, bind, [&](Binder& binder, const int idx) {
            results.emplace_back();
            const auto offset = results.back().left.dbUnbind(binder, idx);
            results.back().right.dbUnbind(binder, offset);
        });
        return results;
    }

    template <typename T, typename V> void remove(const V& id) {
        auto sql = fmt::format("DELETE FROM {} WHERE id = ?;", T::dbName());
        const auto bind = [&](Binder& binder, const int idx) { binder.bind(idx, id); };
        query(sql, bind, nullptr);
    }

    template <typename T> void update(const T& item) {
        const auto sql = fmt::format("UPDATE {} SET {} WHERE id = ?;", T::dbName(), T::dbSets());
        const auto bind = [&](Binder& binder, const int idx) {
            const auto nextIdx = item.dbBind(binder, idx);
            binder.bind(nextIdx, item.id);
        };
        query(sql, bind, nullptr);
    }

    template <typename T, typename... Args> void set(const std::string& sql, Args&&... args) {
        const auto st = fmt::format("UPDATE {} SET {};", T::dbName(), sql);

        const auto bind = [&](Binder& binder, const int idx) { binder.bindArgs(idx, args...); };
        query(st, bind, nullptr);
    }

    template <typename T, typename... Args> size_t count(const std::string& where = "", Args&&... args) {
        const auto sql = fmt::format("SELECT COUNT(*) FROM {} {};", T::dbName(), where);
        size_t count = 0;
        const auto bind = [&](Binder& binder, const int idx) { binder.bindArgs(idx, args...); };
        query(sql, bind, [&](Binder& binder, const int idx) { count = binder.column<uint64_t>(idx); });
        return count;
    }

    void exec(const std::string& sql) const;

    void beginTransaction() const {
        exec("BEGIN TRANSACTION;");
    }
    void endTransaction() const {
        exec("END TRANSACTION;");
    }
    void rollbackTransaction() const {
        exec("ROLLBACK TRANSACTION;");
    }
    uint64_t getLastInsertRowId() const;

    template <typename Fn> void transaction(const Fn& fn) const {
        try {
            beginTransaction();
            fn();
            endTransaction();
        } catch (...) {
            rollbackTransaction();
            std::rethrow_exception(std::current_exception());
        }
    }

    Schema describe(const std::string& name) const;

    template <typename T> bool exists() {
        return exists(T::dbName());
    }

private:
    using StmtPtr = std::unique_ptr<sqlite3_stmt, std::function<void(sqlite3_stmt*)>>;

    StmtPtr prepare(const std::string& sql) const;
    bool exists(const std::string& name) const;
    void createOrMigrate(const Schema& schema) const;
    void migrate(const Schema& old, const Schema& schema) const;
    void create(const Schema& schema) const;
    void init(const std::string& path);
    void query(const std::string& sql, const BinderCallback& bind, const SelectCallback& callback) const;
    std::string getLastError() const;

    std::mutex mutex;
    std::unique_ptr<sqlite3, std::function<void(sqlite3*)>> db;
};

class TransactionGuard {
public:
    explicit TransactionGuard(Database& db) : db(db) {
        db.beginTransaction();
    }

    ~TransactionGuard() {
        db.endTransaction();
    }

private:
    Database& db;
};

using SchemaField = Database::Schema::Field;
// using SchemaIndex = Database::Schema::Index;

template <> struct Database::Helper<int64_t> {
    static void set(sqlite3_stmt* stmt, const int idx, const int64_t& value) {
        Stmt::setInt64(stmt, idx, value);
    }
    static int64_t get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getInt64(stmt, idx);
    }
};

template <> struct Database::Helper<uint64_t> {
    static void set(sqlite3_stmt* stmt, const int idx, const uint64_t& value) {
        Stmt::setIntU64(stmt, idx, value);
    }
    static uint64_t get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getIntU64(stmt, idx);
    }
};

template <> struct Database::Helper<int32_t> {
    static void set(sqlite3_stmt* stmt, const int idx, const int32_t& value) {
        Stmt::setInt32(stmt, idx, value);
    }
    static int32_t get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getInt32(stmt, idx);
    }
};

template <> struct Database::Helper<uint32_t> {
    static void set(sqlite3_stmt* stmt, const int idx, const uint32_t& value) {
        Stmt::setIntU32(stmt, idx, value);
    }
    static uint32_t get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getIntU32(stmt, idx);
    }
};

template <> struct Database::Helper<float> {
    static void set(sqlite3_stmt* stmt, const int idx, const float& value) {
        Stmt::setFloat(stmt, idx, value);
    }
    static float get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getFloat(stmt, idx);
    }
};

template <> struct Database::Helper<double> {
    static void set(sqlite3_stmt* stmt, const int idx, const double& value) {
        Stmt::setDouble(stmt, idx, value);
    }
    static double get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getDouble(stmt, idx);
    }
};

template <> struct Database::Helper<bool> {
    static void set(sqlite3_stmt* stmt, const int idx, const bool& value) {
        Stmt::setBool(stmt, idx, value);
    }
    static bool get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getBool(stmt, idx);
    }
};

template <> struct Database::Helper<std::string> {
    static void set(sqlite3_stmt* stmt, const int idx, const std::string& value) {
        Stmt::setString(stmt, idx, value);
    }
    static std::string get(sqlite3_stmt* stmt, const int idx) {
        return Stmt::getString(stmt, idx);
    }
};

template <> struct Database::Helper<const char*> {
    static void set(sqlite3_stmt* stmt, const int idx, const char* value) {
        Stmt::setConstChar(stmt, idx, value);
    }
    static std::string get(sqlite3_stmt* stmt, const int idx) {
        throw std::runtime_error("Cannot get const char from database, use string instead");
    }
};

template <> struct Database::Helper<Vector3> {
    static void set(sqlite3_stmt* stmt, const int idx, const Vector3& value) {
        Stmt::setBlob(stmt, idx, reinterpret_cast<void*>(const_cast<Vector3*>(&value)), sizeof(value));
    }
    static Vector3 get(sqlite3_stmt* stmt, const int idx) {
        Vector3 vec;
        Stmt::getBlob(stmt, idx, reinterpret_cast<void*>(&vec), sizeof(vec));
        return vec;
    }
};

template <typename T> struct Database::Helper<std::optional<T>> {
    static void set(sqlite3_stmt* stmt, const int idx, const std::optional<T>& value) {
        if (value.has_value()) {
            Helper<T>::set(stmt, idx, value.value());
        } else {
            Stmt::setNull(stmt, idx);
        }
    }

    static std::optional<T> get(sqlite3_stmt* stmt, const int idx) {
        if (Stmt::isNull(stmt, idx)) {
            return std::nullopt;
        } else {
            return {Helper<T>::get(stmt, idx)};
        }
    }
};

template <typename T> struct Database::Helper<std::vector<T>> {
    static void set(sqlite3_stmt* stmt, const int idx, const std::vector<T>& value) {
        std::stringstream ss;
        auto first = true;
        for (const auto& e : value) {
            if (!first) {
                ss << ",";
            }
            ss << e;
            first = false;
        }
        Helper<std::string>::set(stmt, idx, ss.str());
    }

    static std::vector<T> get(sqlite3_stmt* stmt, const int idx) {
        std::vector<T> res;
        const auto tokens = split(Helper<std::string>::get(stmt, idx), ",");
        for (const auto& t : tokens) {
            res.push_back(T{t});
        }
        return res;
    }
};

#define DB_TABLE_NAME(name)                                                                                            \
    static const char* dbName() {                                                                                      \
        return name;                                                                                                   \
    }

#define DB_SCHEMA(...)                                                                                                 \
    static const Database::Schema& dbSchema() {                                                                        \
        static Database::Schema schema{dbName(), __VA_ARGS__};                                                         \
        assert(!schema.fields.empty());                                                                                \
        return schema;                                                                                                 \
    }

static const inline std::array<const char*, 12> DB_VALUES_ARRAY = {
    "",
    "?",
    "?,?",
    "?,?,?",
    "?,?,?,?",
    "?,?,?,?,?",
    "?,?,?,?,?,?",
    "?,?,?,?,?,?,?",
    "?,?,?,?,?,?,?,?",
    "?,?,?,?,?,?,?,?,?",
    "?,?,?,?,?,?,?,?,?,?",
    "?,?,?,?,?,?,?,?,?,?,?",
};

template <typename Arg> static int dbUtilsBindFields(Database::Binder& binder, const int idx, const Arg& value) {
    binder.bind(idx, value);
    return idx + 1;
}

template <typename Arg, typename... Args>
static int dbUtilsBindFields(Database::Binder& binder, const int idx, const Arg& value, const Args&... values) {
    binder.bind(idx, value);
    return dbUtilsBindFields<Args...>(binder, idx + 1, values...);
}

template <typename Arg> static int dbUtilsUnbindFields(Database::Binder& binder, const int idx, Arg& value) {
    value = binder.column<Arg>(idx);
    return idx + 1;
}

template <typename Arg, typename... Args>
static int dbUtilsUnbindFields(Database::Binder& binder, const int idx, Arg& value, Args&... values) {
    value = binder.column<Arg>(idx);
    return dbUtilsUnbindFields<Args...>(binder, idx + 1, values...);
}

SCISSIO_API extern std::string dbUtilsSelectNames(const std::string& table, const std::string& fields);
SCISSIO_API extern std::string dbUtilsFieldNames(const std::string& fields);
SCISSIO_API extern std::string dbUtilsSetNames(const std::string& fields);

#define DB_VALUES_STRING(N) DB_VALUES_ARRAY.at(N)

#define DB_VALUES()                                                                                                    \
    static const std::string& dbValues() {                                                                             \
        static std::string s = DB_VALUES_STRING(dbSchema().fields.size() - 1);                                         \
        return s;                                                                                                      \
    }

#define DB_BIND_FIELDS(...)                                                                                            \
    int dbBind(Database::Binder& binder, const int idx) const {                                                        \
        return dbUtilsBindFields(binder, idx, __VA_ARGS__);                                                            \
    }                                                                                                                  \
    int dbUnbind(Database::Binder& binder, const int idx) {                                                            \
        return dbUtilsUnbindFields(binder, idx, __VA_ARGS__);                                                          \
    }

#define DB_BIND_FIELDS_WITH_ID(...)                                                                                    \
    int dbBind(Database::Binder& binder, const int idx) const {                                                        \
        return dbUtilsBindFields(binder, idx, __VA_ARGS__);                                                            \
    }                                                                                                                  \
    int dbUnbind(Database::Binder& binder, const int idx) {                                                            \
        id = binder.column<decltype(id)>(idx);                                                                         \
        return dbUtilsUnbindFields(binder, idx + 1, __VA_ARGS__);                                                      \
    }

#define DB_FIELD_SELECT(FIELDS)                                                                                        \
    static const std::string& dbSelect() {                                                                             \
        static std::string s = dbUtilsSelectNames(dbName(), FIELDS);                                                   \
        return s;                                                                                                      \
    }

#define DB_FIELD_NAMES(FIELDS)                                                                                         \
    static const std::string& dbNames() {                                                                              \
        static std::string s = dbUtilsFieldNames(FIELDS);                                                              \
        return s;                                                                                                      \
    }

#define DB_FIELD_SETS(FIELDS)                                                                                          \
    static const std::string& dbSets() {                                                                               \
        static std::string s = dbUtilsSetNames(FIELDS);                                                                \
        return s;                                                                                                      \
    }

#define DB_BIND(...)                                                                                                   \
    DB_FIELD_SELECT(#__VA_ARGS__);                                                                                     \
    DB_FIELD_NAMES(#__VA_ARGS__);                                                                                      \
    DB_FIELD_SETS(#__VA_ARGS__);                                                                                       \
    DB_BIND_FIELDS_WITH_ID(__VA_ARGS__);                                                                               \
    DB_VALUES();                                                                                                       \
    MSGPACK_DEFINE_ARRAY(__VA_ARGS__);

} // namespace Scissio
