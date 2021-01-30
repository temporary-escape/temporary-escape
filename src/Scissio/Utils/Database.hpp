#pragma once
#include "../Library.hpp"
#include "../Utils/Path.hpp"

#include <array>
#include <fmt/format.h>
#include <functional>
#include <mutex>
#include <optional>
#include <variant>

struct sqlite3;
struct sqlite3_stmt;

namespace Scissio {
struct TableField {
    static constexpr int Integer = 1 << 0;
    static constexpr int Boolean = 1 << 1;
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

    std::string name;
    int flags{0};
    std::string defval;
    std::string ref;

    explicit TableField(std::string name) : name(std::move(name)) {
    }

    TableField& integer() {
        flags |= Integer;
        return *this;
    }

    TableField& boolean() {
        flags |= Text;
        return *this;
    }

    TableField& real() {
        flags |= Real;
        return *this;
    }

    TableField& text() {
        flags |= Text;
        return *this;
    }

    TableField& primary() {
        flags |= Primary;
        return *this;
    }

    TableField& autoInc() {
        flags |= AutoIncrement;
        return *this;
    }

    TableField& nonNull() {
        flags |= NonNull;
        return *this;
    }

    TableField& onDeleteNull() {
        flags |= OnDeleteNull;
        return *this;
    }

    TableField& onDeleteDefault() {
        flags |= OnDeleteDefault;
        return *this;
    }

    TableField& onDeleteRestrict() {
        flags |= OnDeleteRestrict;
        return *this;
    }

    TableField& onDeleteCascade() {
        flags |= OnDeleteCascade;
        return *this;
    }

    TableField& onUpdateNull() {
        flags |= OnUpdateNull;
        return *this;
    }

    TableField& onUpdateDefault() {
        flags |= OnUpdateDefault;
        return *this;
    }

    TableField& onUpdateRestrict() {
        flags |= OnUpdateRestrict;
        return *this;
    }

    TableField& onUpdateCascade() {
        flags |= OnUpdateCascade;
        return *this;
    }

    TableField& unique() {
        flags |= Unique;
        return *this;
    }

    TableField& indexed() {
        flags |= Indexed;
        return *this;
    }

    template <typename T> TableField& references(const std::string& name) {
        ref = fmt::format("{} ({})", T::dbName(), name);
        flags |= Foreign;
        return *this;
    }

    TableField& default(std::string defval) {
        this->defval = std::move(defval);
        return *this;
    }
};

struct TableSchema {
    std::string name;
    std::vector<TableField> fields;
};

struct SCISSIO_API ColumnUtils {
    static bool isNull(sqlite3_stmt* stmt, int idx);
};

template <typename T> struct SCISSIO_API ColumnBinder {
    static void b(sqlite3_stmt* stmt, int idx, const T& value);
    static T f(sqlite3_stmt* stmt, int idx);
};

template <typename T> struct SCISSIO_API ColumnBinder<std::optional<T>> {
    static void b(sqlite3_stmt* stmt, const int idx, const T& value) {
        if (value.has_value()) {
            ColumnBinder<T>::template b(stmt, idx, value.value());
        } else {
            ColumnBinder<std::nullptr_t>::template b(stmt, idx, nullptr);
        }
    }

    static T f(sqlite3_stmt* stmt, const int idx) {
        if (ColumnUtils::isNull(stmt, idx)) {
            return std::nullopt;
        } else {
            return ColumnBinder<T>::template f(stmt, idx);
        }
    }
};

class SCISSIO_API TableBinder {
public:
    explicit TableBinder(sqlite3_stmt* stmt);
    ~TableBinder();

    template <typename T> void bind(int idx, const T& value) {
        ColumnBinder<T>::template b(stmt, idx, value);
    }

    template <typename T> T column(int idx) {
        return ColumnBinder<T>::template f(stmt, idx);
    }

    void bindArgs(const int idx) {
        (void)idx;
    }

    template <typename Arg> void bindArgs(const int idx, const Arg& arg) {
        ColumnBinder<Arg>::template b(stmt, idx, arg);
    }

    template <typename Arg, typename... Args> void bindArgs(const int idx, const Arg& arg, const Args&... args) {
        ColumnBinder<Arg>::template b(stmt, idx, arg);
        bindArgs<Args...>(idx + 1, args...);
    }

private:
    sqlite3_stmt* stmt;
};

class SCISSIO_API Database {
public:
    using BinderCallback = std::function<void(TableBinder&, int)>;
    using SelectCallback = std::function<void(TableBinder&, int)>;

    explicit Database(const Path& path);
    explicit Database();
    Database(const Database& other) = delete;
    Database(Database&& other);
    virtual ~Database();
    Database& operator=(const Database& other) = delete;
    Database& operator=(Database&& other);
    void swap(Database& other);

    template <typename T> void create() {
        create(T::dbSchema());
    }

    template <typename T> void pragma(const std::string& name, const T& value) {
        const auto sql = fmt::format("PRAGMA {} = {};", name, value);
        query(sql, nullptr, nullptr);
    }

    template <typename T> T pragma(const std::string& name) {
        const auto sql = fmt::format("PRAGMA {};", name);
        T value{};
        query(sql, nullptr, [&](TableBinder& binder, const int idx) { value = binder.column<T>(idx); });
        return value;
    }

    template <typename T, typename V> std::optional<T> get(const V& id) {
        const auto results = select<T>("WHERE id = ?", id);
        if (results.empty()) {
            return std::nullopt;
        }
        return results.at(0);
    }

    template <typename T> uint64_t insert(T& value) {
        // We need to acquire lock because getLastInsertRowId() is not thread safe
        std::lock_guard<std::mutex> lock{mutex};

        const auto sql = fmt::format("INSERT INTO {} ({}) VALUES ({});", T::dbName(), T::dbNames(), T::dbValues());
        BinderCallback bind = std::bind(&T::dbBind, &value, std::placeholders::_1, std::placeholders::_2);
        query(sql, bind, nullptr);
        value.id = getLastInsertRowId();
        return value.id;
    }

    template <typename T, typename... Args> std::vector<T> select(const std::string& where = "", Args&&... args) {
        const auto sql = fmt::format("SELECT id,{} FROM {} {};", T::dbNames(), T::dbName(), where);
        std::vector<T> results;

        const auto bind = [&](TableBinder& binder, const int idx) { binder.bindArgs(idx, args...); };

        query(sql, bind, [&](TableBinder& binder, const int idx) {
            results.emplace_back();
            results.back().dbUnbind(binder, idx);
        });
        return results;
    }

    template <typename T, typename V> void remove(const V& id) {
        auto sql = fmt::format("DELETE FROM {} WHERE id = ?;", T::dbName());
        const auto bind = [&](TableBinder& binder, const int idx) { binder.bind(idx, id); };
        query(sql, bind, nullptr);
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

    template <typename Fn> void transaction(const Fn& fn) {
        try {
            beginTransaction();
            fn();
            endTransaction();
        } catch (...) {
            rollbackTransaction();
            std::rethrow_exception(std::current_exception());
        }
    }

private:
    using StmtPtr = std::unique_ptr<sqlite3_stmt, std::function<void(sqlite3_stmt*)>>;

    StmtPtr prepare(const std::string& sql) const;
    void create(const TableSchema& schema);
    void init(const std::string& path);
    void query(const std::string& sql, const BinderCallback& bind, const SelectCallback& callback);
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

#define DB_TABLE_NAME(name)                                                                                            \
    static const char* dbName() {                                                                                      \
        return name;                                                                                                   \
    }

#define DB_SCHEMA(...)                                                                                                 \
    static const TableSchema& dbSchema() {                                                                             \
        static TableSchema schema{dbName(), __VA_ARGS__};                                                              \
        assert(schema.fields.size() > 0);                                                                              \
        return schema;                                                                                                 \
    }

static const inline std::array<const char*, 8> DB_VALUES_ARRAY = {
    "", "?", "?,?", "?,?,?", "?,?,?,?", "?,?,?,?,?", "?,?,?,?,?,?", "?,?,?,?,?,?,?",
};

template <typename Arg> static void dbUtilsBindFields(TableBinder& binder, const int idx, const Arg& value) {
    binder.bind(idx, value);
}

template <typename Arg, typename... Args>
static void dbUtilsBindFields(TableBinder& binder, const int idx, const Arg& value, const Args&... values) {
    binder.bind(idx, value);
    dbUtilsBindFields<Args...>(binder, idx + 1, values...);
}

template <typename Arg> static void dbUtilsUnbindFields(TableBinder& binder, const int idx, Arg& value) {
    value = binder.column<Arg>(idx);
}

template <typename Arg, typename... Args>
static void dbUtilsUnbindFields(TableBinder& binder, const int idx, Arg& value, Args&... values) {
    value = binder.column<Arg>(idx);
    dbUtilsUnbindFields<Args...>(binder, idx + 1, values...);
}

template <typename Name> static std::string dbUtilsFieldNames(const Name& name) {
    return std::string{name};
}

template <typename Name, typename... Names>
static std::string dbUtilsFieldNames(const Name& name, const Names&... names) {
    return name + dbUtilsFieldNames<Names...>(names...);
}

#define DB_VALUES_STRING(N) DB_VALUES_ARRAY.at(N)

#define DB_VALUES()                                                                                                    \
    static const std::string& dbValues() {                                                                             \
        static std::string s = DB_VALUES_STRING(dbSchema().fields.size() - 1);                                         \
        return s;                                                                                                      \
    }

#define DB_BIND_FIELDS(...)                                                                                            \
    void dbBind(TableBinder& binder, const int idx) {                                                                  \
        dbUtilsBindFields(binder, idx, __VA_ARGS__);                                                                   \
    }                                                                                                                  \
    void dbUnbind(TableBinder& binder, const int idx) {                                                                \
        id = binder.column<decltype(id)>(idx);                                                                         \
        dbUtilsUnbindFields(binder, idx + 1, __VA_ARGS__);                                                             \
    }

#define DB_FIELD_NAMES(...)                                                                                            \
    static const std::string& dbNames() {                                                                              \
        static std::string s = dbUtilsFieldNames(__VA_ARGS__);                                                         \
        return s;                                                                                                      \
    }

#define DB_BIND(...)                                                                                                   \
    DB_FIELD_NAMES(#__VA_ARGS__);                                                                                      \
    DB_BIND_FIELDS(__VA_ARGS__);                                                                                       \
    DB_VALUES();

} // namespace Scissio
