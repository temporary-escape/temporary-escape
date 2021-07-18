#include "Database.hpp"

#include "../Math/Vector.hpp"
#include "../Utils/Exceptions.hpp"

#include <iostream>
#include <set>
#include <sqlite3.h>
#include <sstream>

using namespace Scissio;

bool Database::Stmt::isNull(sqlite3_stmt* stmt, const int idx) {
    return sqlite3_column_type(stmt, idx) == SQLITE_NULL;
}

void Database::Stmt::setInt64(sqlite3_stmt* stmt, const int idx, const int64_t value) {
    sqlite3_bind_int64(stmt, idx, value);
}

void Database::Stmt::setIntU64(sqlite3_stmt* stmt, const int idx, const uint64_t value) {
    sqlite3_bind_int64(stmt, idx, static_cast<int64_t>(value));
}

void Database::Stmt::setInt32(sqlite3_stmt* stmt, const int idx, const int32_t value) {
    sqlite3_bind_int(stmt, idx, value);
}

void Database::Stmt::setIntU32(sqlite3_stmt* stmt, const int idx, const uint32_t value) {
    sqlite3_bind_int(stmt, idx, static_cast<int32_t>(value));
}

void Database::Stmt::setFloat(sqlite3_stmt* stmt, const int idx, const float value) {
    sqlite3_bind_double(stmt, idx, static_cast<double>(value));
}

void Database::Stmt::setDouble(sqlite3_stmt* stmt, const int idx, const double value) {
    sqlite3_bind_double(stmt, idx, static_cast<double>(value));
}

void Database::Stmt::setBool(sqlite3_stmt* stmt, const int idx, const bool value) {
    sqlite3_bind_int(stmt, idx, static_cast<int>(value));
}

void Database::Stmt::setString(sqlite3_stmt* stmt, const int idx, const std::string& value) {
    sqlite3_bind_text(stmt, idx, value.c_str(), static_cast<int>(value.size()), nullptr);
}

void Database::Stmt::setConstChar(sqlite3_stmt* stmt, const int idx, const char* value) {
    sqlite3_bind_text(stmt, idx, value, static_cast<int>(std::strlen(value)), nullptr);
}

void Database::Stmt::setNull(sqlite3_stmt* stmt, const int idx) {
    sqlite3_bind_null(stmt, idx);
}

int64_t Database::Stmt::getInt64(sqlite3_stmt* stmt, const int idx) {
    return sqlite3_column_int64(stmt, idx);
}

uint64_t Database::Stmt::getIntU64(sqlite3_stmt* stmt, const int idx) {
    return static_cast<uint64_t>(sqlite3_column_int64(stmt, idx));
}

int32_t Database::Stmt::getInt32(sqlite3_stmt* stmt, const int idx) {
    return sqlite3_column_int(stmt, idx);
}

uint32_t Database::Stmt::getIntU32(sqlite3_stmt* stmt, const int idx) {
    return static_cast<uint32_t>(sqlite3_column_int(stmt, idx));
}

float Database::Stmt::getFloat(sqlite3_stmt* stmt, const int idx) {
    return static_cast<float>(sqlite3_column_double(stmt, idx));
}

double Database::Stmt::getDouble(sqlite3_stmt* stmt, const int idx) {
    return sqlite3_column_double(stmt, idx);
}

bool Database::Stmt::getBool(sqlite3_stmt* stmt, const int idx) {
    return static_cast<bool>(sqlite3_column_int(stmt, idx));
}

std::string Database::Stmt::getString(sqlite3_stmt* stmt, const int idx) {
    const auto* str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, idx));
    if (str) {
        return std::string(str);
    }
    return "";
}

void Database::Stmt::setBlob(sqlite3_stmt* stmt, const int idx, void* data, const size_t length) {
    sqlite3_bind_blob(stmt, idx, data, static_cast<int>(length), SQLITE_TRANSIENT);
}

/*std::vector<char> Database::Stmt::getBlob(sqlite3_stmt* stmt, int idx) {
    std::vector<char> res;
    const auto length = sqlite3_column_bytes(stmt, idx);
    if (length > 0) {
        res.resize(length);
        const auto buff = sqlite3_column_blob(stmt, idx);
        std::memcpy(res.data(), buff, length);
    }
    return res;
}*/

void Database::Stmt::getBlob(sqlite3_stmt* stmt, const int idx, void* dst, const size_t length) {
    const auto test = sqlite3_column_bytes(stmt, idx);
    if (length == test) {
        const auto buff = sqlite3_column_blob(stmt, idx);
        std::memcpy(dst, buff, length);
    }
}

Database::Binder::Binder(sqlite3_stmt* stmt) : stmt(stmt) {
}

Database::Binder::~Binder() = default;

Database::Database(const Path& path) {
    init(path.string());
}

Database::Database() {
    init(":memory:");
}

Database::~Database() = default;

void Database::init(const std::string& path) {
    if (sqlite3_threadsafe() == 0) {
        EXCEPTION("Sqlite3 is not threadsafe!");
    }

    sqlite3* temp = nullptr;
    if (sqlite3_open(path.c_str(), &temp)) {
        if (temp) {
            const std::string msg = sqlite3_errmsg(temp);
            sqlite3_close(temp);
            EXCEPTION("Failed to open database path: '{}' error: {}", path, msg);
        } else {
            EXCEPTION("Failed to open database path: '{}' error: unknown", path);
        }
    }

    db = std::unique_ptr<sqlite3, std::function<void(sqlite3*)>>(temp, [](sqlite3* ptr) { sqlite3_close(ptr); });

    pragma("foreign_keys", "ON");
}

Database::Database(Database&& other) {
    swap(other);
}

Database& Database::operator=(Database&& other) {
    if (this != &other) {
        swap(other);
    }
    return *this;
}

void Database::swap(Database& other) {
    std::unique_lock<std::mutex> llock{mutex, std::defer_lock};
    std::unique_lock<std::mutex> rlock{other.mutex, std::defer_lock};
    std::lock(llock, rlock);
    std::swap(db, other.db);
}

Database::StmtPtr Database::prepare(const std::string& sql) const {
    sqlite3_stmt* temp = nullptr;

    Log::d("{}", sql);

    const auto r = sqlite3_prepare(db.get(), sql.c_str(), -1, &temp, nullptr);
    if (r != SQLITE_OK) {
        if (temp) {
            sqlite3_finalize(temp);
        }
        EXCEPTION("Failed to prepare SQL statement: '{}' error: {}", sql, getLastError());
    }

    auto stmt = StmtPtr(temp, [](sqlite3_stmt* stmt) { sqlite3_finalize(stmt); });

    return stmt;
}

void Database::query(const std::string& sql, const BinderCallback& bind, const SelectCallback& callback) const {
    auto stmt = prepare(sql);

    Binder binder(stmt.get());

    if (bind) {
        bind(binder, 1);
    }

    while (true) {
        const auto rc = sqlite3_step(stmt.get());

        if (rc == SQLITE_DONE) {
            break;
        }

        if (rc == SQLITE_ERROR) {
            stmt.reset();
            EXCEPTION("Failed to execute SQL: '{}' error: {}", sql, getLastError());
        }

        if (rc == SQLITE_ROW && callback) {
            callback(binder, 0);
        }
    }
}

uint64_t Database::getLastInsertRowId() const {
    return sqlite3_last_insert_rowid(db.get());
}

static const char* dataType(const SchemaField& field) {
    if (field.flags & Database::Flags::Integer) {
        return "INTEGER";
    }
    if (field.flags & Database::Flags::Text) {
        return "TEXT";
    }
    if (field.flags & Database::Flags::Real) {
        return "REAL";
    }
    if (field.flags & Database::Flags::Binary) {
        return "BLOB";
    }
    EXCEPTION("Can not deduce table field type from flags: {}", field.flags);
}

void Database::createOrMigrate(const Schema& schema) const {
    try {
        if (!exists(schema.name)) {
            create(schema);
            return;
        }

        const auto old = describe(schema.name);
        if (old != schema) {
            migrate(old, schema);
        }
    } catch (...) {
        EXCEPTION_NESTED("Failed to create schema: '{}'", schema.name);
    }
}

static std::string createTableString(const std::string& tableName, const Database::Schema& schema,
                                     const bool temp = false) {
    std::stringstream ss;

    ss << "CREATE";
    if (temp) {
        ss << " TEMPORARY";
    }
    ss << " TABLE " << tableName << "(\n";

    auto first = true;
    for (const auto& field : schema.fields) {
        if (!first) {
            ss << ",\n";
        }
        ss << field.name << " " << dataType(field);
        if (field.flags & Database::Flags::Primary) {
            ss << " PRIMARY KEY";
        }
        if (field.flags & Database::Flags::AutoIncrement) {
            ss << " AUTOINCREMENT";
        }
        if (field.flags & Database::Flags::NonNull) {
            ss << " NOT NULL";
        }
        if (!field.def.empty()) {
            ss << " DEFAULT " << field.def;
        }
        first = false;
    }

    for (const auto& field : schema.fields) {
        if (field.flags & Database::Flags::Foreign) {
            ss << ",\n";
            ss << "FOREIGN KEY (" << field.name << ")";
            ss << " REFERENCES " << field.ref;
            ss << " ON UPDATE";

            if (field.flags & Database::Flags::OnUpdateNull) {
                ss << " SET NULL";
            } else if (field.flags & Database::Flags::OnUpdateDefault) {
                ss << " SET DEFAULT";
            } else if (field.flags & Database::Flags::OnUpdateRestrict) {
                ss << " RESTRICT";
            } else if (field.flags & Database::Flags::OnUpdateCascade) {
                ss << " CASCADE";
            } else {
                ss << " NO ACTION";
            }

            ss << " ON DELETE";
            if (field.flags & Database::Flags::OnDeleteNull) {
                ss << " SET NULL";
            } else if (field.flags & Database::Flags::OnDeleteDefault) {
                ss << " SET DEFAULT";
            } else if (field.flags & Database::Flags::OnDeleteRestrict) {
                ss << " RESTRICT";
            } else if (field.flags & Database::Flags::OnDeleteCascade) {
                ss << " CASCADE";
            } else {
                ss << " NO ACTION";
            }
        }
    }

    ss << "\n);\n";

    for (const auto& field : schema.fields) {
        if (field.flags & Database::Flags::Indexed || field.flags & Database::Flags::Unique) {
            ss << "CREATE";
            if (field.flags & Database::Flags::Unique) {
                ss << " UNIQUE";
            }
            ss << " INDEX IF NOT EXISTS";
            ss << " " << tableName << "_" << field.name << "_idx";
            ss << " ON";
            ss << " " << tableName << "(" << field.name << ");\n";
        }
    }

    return ss.str();
}

static std::string getSchemasCommonFields(const Database::Schema& old, const Database::Schema& schema) {
    std::set<std::string> fieldsOld;
    std::set<std::string> fieldsNew;

    for (const auto& field : old.fields) {
        fieldsOld.insert(field.name);
    }
    for (const auto& field : schema.fields) {
        fieldsNew.insert(field.name);
    }

    const auto& a = old.fields;
    const auto& b = schema.fields;
    std::vector<std::string> common;

    std::set_intersection(fieldsOld.begin(), fieldsOld.end(), fieldsNew.begin(), fieldsNew.end(),
                          std::back_inserter(common));

    std::stringstream ss;
    auto first = true;
    for (const auto& c : common) {
        if (!first) {
            ss << ",";
        }
        ss << c;
        first = false;
    }

    return ss.str();
}

void Database::migrate(const Schema& old, const Schema& schema) const {
    const auto common = getSchemasCommonFields(old, schema);

    std::stringstream ss;
    ss << "PRAGMA foreign_keys=off;\n";

    ss << createTableString("temp", schema, true);
    ss << "INSERT INTO temp (" << common << ") SELECT " << common << " FROM " << schema.name << ";\n";
    ss << "DROP TABLE " << schema.name << ";\n";

    ss << createTableString(schema.name, schema, false);
    ss << "INSERT INTO " << schema.name << " (" << common << ") SELECT " << common << " FROM temp;\n";
    ss << "DROP TABLE temp;\n";

    ss << "PRAGMA foreign_keys=on;\n";
    const auto sql = ss.str();

    try {
        transaction([&]() { exec(sql); });
    } catch (...) {
        Log::w("Table migration:\n{}", sql);
        EXCEPTION_NESTED("Failed to migrate table: '{}'", schema.name);
    }
}

void Database::create(const Schema& schema) const {
    const auto sql = createTableString(schema.name, schema, false);

    try {
        exec(sql);
    } catch (...) {
        Log::w("Table schema:\n{}", sql);
        EXCEPTION_NESTED("Failed to create table '{}'", schema.name);
    }
}

void Database::exec(const std::string& sql) const {
    char* err = nullptr;
    const auto ret = sqlite3_exec(db.get(), sql.c_str(), nullptr, nullptr, &err);
    if (ret != SQLITE_OK) {
        EXCEPTION("SQL exec error: {}", err ? err : "unknown");
    }
}

std::string Database::getLastError() const {
    return sqlite3_errmsg(db.get());
}

struct TableInfo {
    int cid{0};
    std::string name;
    std::string type;
    bool notnull{false};
    std::optional<std::string> dflt_value;
    bool pk{false};

    DB_BIND_FIELDS(cid, name, type, notnull, dflt_value, pk);
};

struct ForeignKeyInfo {
    int id{0};
    int seq{0};
    std::string table;
    std::string from;
    std::string to;
    std::string on_update;
    std::string on_delete;
    std::string match;

    DB_BIND_FIELDS(id, seq, table, from, to, on_update, on_delete, match);
};

struct IndexInfo {
    int seq{0};
    std::string name;
    bool unique{false};
    std::string origin;
    bool partial{false};

    DB_BIND_FIELDS(seq, name, unique, origin, partial);
};

static SchemaField parse(const TableInfo& info) {
    SchemaField field(info.name);

    if (info.pk) {
        field.primary();
        field.autoInc();
    }

    if (info.notnull) {
        field.nonNull();
    }

    if (info.dflt_value.has_value()) {
        field.defval(info.dflt_value.value());
    }

    if (info.type == "TEXT") {
        field.text();
    }

    if (info.type == "REAL") {
        field.real();
    }

    if (info.type == "INTEGER") {
        field.integer();
    }

    if (info.type == "BLOB") {
        field.binary();
    }

    return field;
}

bool Database::exists(const std::string& name) const {
    auto sql = fmt::format("PRAGMA table_info(\'{}\');", name);
    std::vector<TableInfo> results;
    query(sql, nullptr, [&](Binder& binder, const int idx) {
        results.emplace_back();
        results.back().dbUnbind(binder, idx);
    });

    return !results.empty();
}

Database::Schema Database::describe(const std::string& name) const {
    auto sql = fmt::format("PRAGMA table_info(\'{}\');", name);
    std::vector<TableInfo> results;
    query(sql, nullptr, [&](Binder& binder, const int idx) {
        results.emplace_back();
        results.back().dbUnbind(binder, idx);
    });

    if (results.empty()) {
        EXCEPTION("Failed to describe table schema of: '{}' error: table does not exist", name);
    }

    std::vector<SchemaField> fields;
    for (const auto& result : results) {
        fields.push_back(parse(result));
    }

    sql = fmt::format("PRAGMA foreign_key_list(\'{}\');", name);
    std::vector<ForeignKeyInfo> foreigns;
    query(sql, nullptr, [&](Binder& binder, const int idx) {
        foreigns.emplace_back();
        foreigns.back().dbUnbind(binder, idx);
    });

    for (const auto& key : foreigns) {
        auto it = std::find_if(fields.begin(), fields.end(), [&](const SchemaField& f) { return f.name == key.from; });
        if (it == fields.end()) {
            EXCEPTION("Unable to find foreign key field: '{}' in table scema of: '{}'", key.from, name);
        }

        auto& field = *it;
        field.flags |= Flags::Foreign;
        field.ref = fmt::format("{} ({})", key.table, key.to);

        if (key.on_delete == "CASCADE") {
            field.flags |= Flags::OnDeleteCascade;
        }
        if (key.on_delete == "SET NULL") {
            field.flags |= Flags::OnDeleteNull;
        }
        if (key.on_delete == "SET DEFAULT") {
            field.flags |= Flags::OnDeleteDefault;
        }
        if (key.on_delete == "RESTRICT") {
            field.flags |= Flags::OnDeleteRestrict;
        }

        if (key.on_update == "CASCADE") {
            field.flags |= Flags::OnUpdateCascade;
        }
        if (key.on_update == "SET NULL") {
            field.flags |= Flags::OnUpdateNull;
        }
        if (key.on_update == "SET DEFAULT") {
            field.flags |= Flags::OnUpdateDefault;
        }
        if (key.on_update == "RESTRICT") {
            field.flags |= Flags::OnUpdateRestrict;
        }
    }

    sql = fmt::format("PRAGMA index_list(\'{}\');", name);
    std::vector<IndexInfo> indexes;
    query(sql, nullptr, [&](Binder& binder, const int idx) {
        indexes.emplace_back();
        indexes.back().dbUnbind(binder, idx);
    });

    for (const auto& index : indexes) {
        auto it = std::find_if(fields.begin(), fields.end(), [&](const SchemaField& f) {
            return index.name == fmt::format("{}_{}_idx", name, f.name);
        });
        if (it == fields.end()) {
            EXCEPTION("Unable to find index key field: '{}' in table scema of: '{}'", index.name, name);
        }

        auto& field = *it;
        field.indexed();

        if (index.unique) {
            field.unique();
        }
    }

    return {
        name,
        std::move(fields),
    };
}

static std::string strip(const std::string& s) {
    if (s.at(0) == ' ') {
        return s.substr(1);
    }
    return s;
}

std::string Scissio::dbUtilsSelectNames(const std::string& table, const std::string& fields) {
    const auto tokens = split(fields, ",");
    auto first = true;
    std::stringstream ss;
    for (const auto& token : tokens) {
        if (!first) {
            ss << ",";
        }
        ss << table << "." << strip(token);
        first = false;
    }
    return ss.str();
}

std::string Scissio::dbUtilsFieldNames(const std::string& fields) {
    const auto tokens = split(fields, ",");
    auto first = true;
    std::stringstream ss;
    for (const auto& token : tokens) {
        if (!first) {
            ss << ",";
        }
        ss << strip(token);
        first = false;
    }
    return ss.str();
}

std::string Scissio::dbUtilsSetNames(const std::string& fields) {
    const auto tokens = split(fields, ",");
    auto first = true;
    std::stringstream ss;
    for (const auto& token : tokens) {
        if (!first) {
            ss << ",";
        }
        ss << strip(token) << " = ?";
        first = false;
    }
    return ss.str();
}
