#include "Database.hpp"
#include "../Utils/Exceptions.hpp"

#include <sqlite3.h>
#include <sstream>

using namespace Scissio;

TableBinder::TableBinder(sqlite3_stmt* stmt) : stmt(stmt) {
}

TableBinder::~TableBinder() = default;

template <> void ColumnBinder<uint64_t>::b(sqlite3_stmt* stmt, const int idx, const uint64_t& value) {
    sqlite3_bind_int64(stmt, idx, static_cast<int64_t>(value));
}

template <> void ColumnBinder<int64_t>::b(sqlite3_stmt* stmt, const int idx, const int64_t& value) {
    sqlite3_bind_int64(stmt, idx, value);
}

template <> void ColumnBinder<uint32_t>::b(sqlite3_stmt* stmt, const int idx, const uint32_t& value) {
    sqlite3_bind_int(stmt, idx, static_cast<int64_t>(value));
}

template <> void ColumnBinder<int32_t>::b(sqlite3_stmt* stmt, const int idx, const int32_t& value) {
    sqlite3_bind_int(stmt, idx, value);
}

template <> void ColumnBinder<std::string>::b(sqlite3_stmt* stmt, const int idx, const std::string& value) {
    sqlite3_bind_text(stmt, idx, value.c_str(), static_cast<int>(value.size()), nullptr);
}

template <> void ColumnBinder<bool>::b(sqlite3_stmt* stmt, const int idx, const bool& value) {
    sqlite3_bind_int(stmt, idx, static_cast<int>(value));
}

template <> void ColumnBinder<float>::b(sqlite3_stmt* stmt, const int idx, const float& value) {
    sqlite3_bind_double(stmt, idx, static_cast<double>(value));
}

template <> void ColumnBinder<double>::b(sqlite3_stmt* stmt, const int idx, const double& value) {
    sqlite3_bind_double(stmt, idx, value);
}

template <> void ColumnBinder<std::nullptr_t>::b(sqlite3_stmt* stmt, const int idx, const std::nullptr_t& value) {
    sqlite3_bind_null(stmt, idx);
}

template <> uint64_t ColumnBinder<uint64_t>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<uint64_t>(sqlite3_column_int64(stmt, idx));
}

template <> int64_t ColumnBinder<int64_t>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<int64_t>(sqlite3_column_int64(stmt, idx));
}

template <> uint32_t ColumnBinder<uint32_t>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<uint64_t>(sqlite3_column_int(stmt, idx));
}

template <> int32_t ColumnBinder<int32_t>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<int64_t>(sqlite3_column_int(stmt, idx));
}

template <> std::string ColumnBinder<std::string>::f(sqlite3_stmt* stmt, const int idx) {
    const auto* str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, idx));
    if (str) {
        return std::string(str);
    } else {
        return "";
    }
}

template <> bool ColumnBinder<bool>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<bool>(sqlite3_column_int64(stmt, idx));
}

template <> float ColumnBinder<float>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<float>(sqlite3_column_double(stmt, idx));
}

template <> double ColumnBinder<double>::f(sqlite3_stmt* stmt, const int idx) {
    return static_cast<double>(sqlite3_column_double(stmt, idx));
}

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

void Database::query(const std::string& sql, const BinderCallback& bind, const SelectCallback& callback) {
    auto stmt = prepare(sql);

    TableBinder binder(stmt.get());

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

static const char* dataType(const TableField& field) {
    if (field.flags & TableField::Integer) {
        return "INTEGER";
    }
    if (field.flags & TableField::Text) {
        return "TEXT";
    }
    if (field.flags & TableField::Real) {
        return "REAL";
    }
    if (field.flags & TableField::Binary) {
        return "BLOB";
    }
    EXCEPTION("Can not deduce table field type from flags: {}", field.flags);
}

void Database::create(const TableSchema& schema) {
    std::stringstream ss;
    ss << "CREATE TABLE " << schema.name << "(\n";
    auto first = true;
    for (const auto& field : schema.fields) {
        if (!first) {
            ss << ",\n";
        }
        ss << field.name << " " << dataType(field);
        if (field.flags & TableField::Primary) {
            ss << " PRIMARY KEY";
        }
        if (field.flags & TableField::AutoIncrement) {
            ss << " AUTOINCREMENT";
        }
        if (field.flags & TableField::NonNull) {
            ss << " NOT NULL";
        }
        if (!field.defval.empty()) {
            ss << " DEFAULT " << field.defval;
        }
        first = false;
    }

    for (const auto& field : schema.fields) {
        if (field.flags & TableField::Foreign) {
            ss << ",\n";
            ss << "FOREIGN KEY (" << field.name << ")";
            ss << " REFERENCES " << field.ref;
            ss << " ON UPDATE";

            if (field.flags & TableField::OnUpdateNull) {
                ss << " SET NULL";
            } else if (field.flags & TableField::OnUpdateDefault) {
                ss << " SET DEFAULT";
            } else if (field.flags & TableField::OnUpdateRestrict) {
                ss << " RESTRICT";
            } else if (field.flags & TableField::OnUpdateCascade) {
                ss << " CASCADE";
            } else {
                ss << " NO ACTION";
            }

            ss << " ON DELETE";
            if (field.flags & TableField::OnDeleteNull) {
                ss << " SET NULL";
            } else if (field.flags & TableField::OnDeleteDefault) {
                ss << " SET DEFAULT";
            } else if (field.flags & TableField::OnDeleteRestrict) {
                ss << " RESTRICT";
            } else if (field.flags & TableField::OnDeleteCascade) {
                ss << " CASCADE";
            } else {
                ss << " NO ACTION";
            }
        }
    }

    ss << "\n);\n";

    for (const auto& field : schema.fields) {
        if (field.flags & TableField::Indexed || field.flags & TableField::Unique) {
            ss << "CREATE";
            if (field.flags & TableField::Unique) {
                ss << " UNIQUE";
            }
            ss << " INDEX IF NOT EXISTS";
            ss << " " << schema.name << "_" << field.name << "_idx";
            ss << " ON";
            ss << " " << schema.name << "(" << field.name << ");\n";
        }
    }

    const auto sql = ss.str();

    try {
        exec(sql);
    } catch (...) {
        Log::d("Table schema:\n{}", sql);
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
